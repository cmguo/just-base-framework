// BackTraceLinux.h

#include <execinfo.h>

namespace framework
{
    namespace debuging
    {

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

    } // namespace memory
} // namespace framework
