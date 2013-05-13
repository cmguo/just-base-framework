// ThreadContext.cpp 

#include "framework/Framework.h"
#include "framework/logger/Context.h"
#include "framework/logger/Module.h"
#include "framework/logger/Buffer.h"

#ifdef BOOST_WINDOWS_API
#else
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#define gettid() pthread_self()
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

            buffers_[mi_time].buf = time_str;
            buffers_[mi_time].len = strlen(time_str);
            buffers_[mi_pid].buf = pid_buffer_;
            buffers_[mi_pid].len = strlen(pid_buffer_);
            buffers_[mi_level].buf = NULL;
            buffers_[mi_level].len = level_str_len;
            buffers_[mi_module].buf = NULL;
            buffers_[mi_module].len = 0;
            buffers_[mi_msg].buf = msg_buffer_;
            buffers_[mi_msg].len = sizeof(msg_buffer_);
            buffers_[mi_other].buf = NULL;
            buffers_[mi_other].len = 0;
        }

        void Context::reset(
            Module const & module, 
            LevelEnum level)
        {
            buffers_[mi_level].buf = level_str[level];
            buffers_[mi_module].buf = module.short_name_;
            buffers_[mi_module].len = module.name_size_;
        }

    } // namespace logger
} // namespace framework
