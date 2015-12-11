// ThreadContext.cpp 

#include "framework/Framework.h"
#include "framework/logger/Context.h"
#include "framework/logger/Module.h"
#include "framework/logger/Buffer.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <stdio.h>
#  include <sys/types.h>
#  include <sys/syscall.h>
#  include <unistd.h>
#  ifndef SYS_gettid
#    define SYS_gettid __NR_gettid
#  endif
#  define gettid() syscall(SYS_gettid)
#endif

#include <sstream>

namespace framework
{
    namespace logger
    {

        void init_pid_buffer(
            char const * fmt, 
            char * buf, 
            size_t len)
        {
            Buffer b(buf, len); // 里面会少用一个字节，我们用来填'\0'
            std::ostream os(&b);
#ifdef BOOST_WINDOWS_API
            unsigned long pid = ::GetCurrentProcessId();
            unsigned long tid = ::GetCurrentThreadId();
#else
            pid_t pid = getpid();
            pid_t tid = (pid_t)gettid();
#endif
            for (char const * p = fmt; *p; ++p) {
                if (*p == '%') {
                    ++p;
                    if (*p == 'p') {
                        os << pid;
                    } else if (*p == 't') {
                        os << tid;
                    } else {
                        os << *p;
                    }
                } else {
                    os << *p;
                }
            }
            os << ' ';
            buf[b.size()] = '\0';
        }

        Context::Context(
            char const * id_fmt, 
            char const * time_str)
        {
            init_pid_buffer(id_fmt, pid_buffer_, sizeof(pid_buffer_));

            buffers_[mi_time] = boost::asio::buffer(time_str, strlen(time_str));
            buffers_[mi_pid] = boost::asio::buffer(pid_buffer_, strlen(pid_buffer_));
            buffers_[mi_msg] = boost::asio::buffer(msg_buffer_, sizeof(msg_buffer_));
        }

        void Context::reset(
            Module const & module, 
            LevelEnum level)
        {
            buffers_[mi_level] = boost::asio::buffer(level_str[level], level_str_len);
            buffers_[mi_module] = boost::asio::buffer(module.short_name_, module.name_size_);
        }

    } // namespace logger
} // namespace framework
