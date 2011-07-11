// PageAlloc.cpp

#include "framework/Framework.h"
#include "framework/memory/PrivateMemory.h"
#include "framework/system/ErrorCode.h"
#include "framework/logger/LoggerFormatRecord.h"

#include <stdio.h>

#ifdef BOOST_WINDOWS_API
#  include <Windows.h>
#else
#  include <unistd.h>
#  include <sys/mman.h>
#  ifndef MAP_ANONYMOUS
#    define MAP_ANONYMOUS MAP_ANON
#  endif
#endif

FRAMEWORK_LOGGER_DECLARE_MODULE("PrivateMemory");

namespace framework
{

    namespace memory
    {

        void * PrivateMemory::alloc_block(
            size_t size)
        {
#ifdef _WIN32
            void * addr = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
#else
            void * addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (addr == MAP_FAILED) {
                addr = NULL;
            }
            if (addr) {
                char * p = (char *)addr;
                char * pe = p + size;
                size_t n = page_size();
                while (p < pe) {
                    *p = '\0';
                    p += n;
                }
            }
#endif
            if (addr == NULL) {
                LOG_F(framework::logger::Logger::kLevelAlarm, 
                    ("alloc page failed"));
            }
            return addr;
        }

        void PrivateMemory::free_block(
            void * addr, 
            size_t size)
        {
#ifdef _WIN32
            VirtualFree(addr, 0, MEM_RELEASE);
#else
            munmap(addr, size);
#endif
        }

    }
}
