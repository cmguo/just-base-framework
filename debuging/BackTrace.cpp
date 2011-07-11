// MemoryPoolDebug.cpp

#include "framework/Framework.h"
#include "framework/system/BytesOrder.h"
#include "framework/debuging/detail/SymbolBuffer.h"

#ifdef BOOST_WINDOWS_API
#  include <dbghelp.h>
#  pragma comment(lib, "dbghelp.lib")
#else
#  include <execinfo.h>
#endif

namespace framework
{
    namespace debuging
    {

#ifdef BOOST_WINDOWS_API

        bool back_trace_init()
        {
            return ::SymInitialize(
                ::GetCurrentProcess(), 
                NULL, 
                TRUE) == TRUE;
        }

        size_t back_trace(
            void ** addrs, 
            size_t num)
        {
            STACKFRAME64 StackFrame;
            ZeroMemory(&StackFrame, sizeof(StackFrame));
            StackFrame.AddrPC.Mode = AddrModeFlat;
            StackFrame.AddrStack.Mode = AddrModeFlat;
            StackFrame.AddrFrame.Mode = AddrModeFlat;
            int i;
            __asm {
                push eax;
label:
                mov eax, label;
                mov i, eax;
                pop eax;
            }
            StackFrame.AddrPC.Offset = i;
            __asm mov i, ebp;
            StackFrame.AddrFrame.Offset = i;
            __asm mov i, esp;
            StackFrame.AddrStack.Offset = i;
            for (size_t n = 0; n < num; ++n) {
                if (!StackWalk64(
                    IMAGE_FILE_MACHINE_I386, 
                    ::GetCurrentProcess(), 
                    ::GetCurrentThread(), 
                    &StackFrame, 
                    NULL, 
                    NULL, 
                    SymFunctionTableAccess64, 
                    SymGetModuleBase64, 
                    NULL)) {
                        addrs[n] = NULL;
                        return n;
                }
                addrs[n] = (void *)StackFrame.AddrPC.Offset;
            }
            return num;
        }

        char ** back_trace_symbols(
            void *const * addrs, 
            size_t num)
        {
            detail::SymbolBuffer buffer(num);
            for (size_t i = 0; i < num; ++i) {
                buffer.next();

                buffer.push('#');
                buffer.push(i + 1);
                buffer.push(" 0x");
                size_t addr = framework::system::BytesOrder::host_to_big_endian((size_t)addrs[i]);
                buffer.push_hex(&addr, sizeof(size_t), 0);

                SYMBOL_INFO_PACKAGE Symbol;
                Symbol.si.SizeOfStruct = sizeof(SYMBOL_INFO);
                Symbol.si.MaxNameLen = sizeof(Symbol) - sizeof(SYMBOL_INFO);
                DWORD64 Displacement = 0;
                if (::SymFromAddr(
                    ::GetCurrentProcess(), 
                    (DWORD64)addrs[i], 
                    &Displacement, 
                    &Symbol.si) 
                    && Symbol.si.Name[0]) {
                        buffer.push(" in ");
                        buffer.push(Symbol.si.Name);
                } else {
                    buffer.push(" in ?");
                }

                IMAGEHLP_LINE64 LineInfo;
                LineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                DWORD Displacement2 = 0;
                if (SymGetLineFromAddr64(
                    ::GetCurrentProcess(), 
                    (DWORD64)addrs[i], 
                    &Displacement2, 
                    &LineInfo)) {
                        buffer.push(" at ");
                        buffer.push(LineInfo.FileName);
                        buffer.push(':');
                        buffer.push((size_t)LineInfo.LineNumber);
                } else {
                    buffer.push(" at ??:0");
                }
            }
            return buffer.detach();
        }

        void release_symbols(
            char ** ptr)
        {
            detail::SymbolBuffer buffer(ptr);
        }

        void back_trace_symbols_fd(
            void *const * addrs, 
            size_t num, 
            int fd)
        {
        }

#else // BOOST_WINDOWS_API

        bool back_trace_init()
        {
            return true;
        }

        size_t back_trace(
            void ** addrs, 
            size_t num)
        {
            return ::backtrace(addrs, num);
        }

        char ** back_trace_symbols(
            void *const * addrs, 
            size_t num)
        {
            return ::backtrace_symbols(addrs, num);
        }

        void release_symbols(
            char ** ptr)
        {
            ::free(ptr);
        }

        void back_trace_symbols_fd(
            void *const * addrs, 
            size_t num, 
            int fd)
        {
            ::backtrace_symbols_fd(addrs, num, fd);
        }

#endif // BOOST_WINDOWS_API

    } // namespace memory
} // namespace framework
