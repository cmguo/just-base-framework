// Context.h 

#ifndef _FRAMEWORK_LOGGER_CONTEXT_H_
#define _FRAMEWORK_LOGGER_CONTEXT_H_

#include "framework/logger/Level.h"
#include "framework/logger/Message.h"
#include "framework/logger/Stream.h"

namespace framework
{
    namespace logger
    {

        class Module;

        class Context
        {
        public:
            typedef Stream::buffer_t buffer_t;

        public:
            Context(
                char const * id_fmt, 
                char const * time_str);

        public:
            void reset(
                Module const & module, 
                LevelEnum level);

            void stack_push();

            void stack_pop();

        public:
            buffer_t const * buffers()
            {
                return buffers_;
            }

            char * msg_buf()
            {
                return msg_buffer_;
            }

            size_t msg_len()
            {
                return sizeof(msg_buffer_);
            }

            void msg_len(
                size_t len)
            {
                buffers_[mi_msg].len = len;
            }

            buffer_t & operator[](
                MessageItemEnum idx)
            {
                return buffers_[idx];
            }

        private:
            size_t pid_size_;
            char pid_buffer_[32];
            char msg_buffer_[1024];
            buffer_t buffers_[mi_max];
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_CONTEXT_H_
