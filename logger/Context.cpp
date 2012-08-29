// ThreadContext.cpp 

#include "framework/Framework.h"
#include "framework/logger/Context.h"
#include "framework/logger/Module.h"

#ifdef BOOST_WINDOWS_API
#else
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#define gettid() pthread_self()
#endif

namespace framework
{
    namespace logger
    {

        void init_pid_buffer(
            char * buf, 
            size_t len)
        {
#ifdef BOOST_WINDOWS_API
            unsigned long pid = ::GetCurrentProcessId();
            unsigned long tid = ::GetCurrentThreadId();
            sprintf_s(buf, len, "[%ld] [%ld] ", pid, tid);
#else
            pid_t pid = getpid();
            unsigned long tid = gettid();
            snprintf(buf, len, "[%d] [%lu] ", pid, tid);
#endif
        }

        Context::Context(
            char const * time_str)
        {
            init_pid_buffer(pid_buffer_, sizeof(pid_buffer_));

#ifndef BOOST_WINDOWS_API
            buffers_[mi_color].buf = color_str[0];
            buffers_[mi_color].len = color_str_len[0];
#endif
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
#ifndef BOOST_WINDOWS_API
            buffers_[mi_color2].buf = color_str[0];
            buffers_[mi_color2].len = color_str_len[0];
#endif
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
