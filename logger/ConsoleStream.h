// ConsoleStream.h

#ifndef _FRAMEWORK_LOGGER_CONSOLE_STREAM_H_
#define _FRAMEWORK_LOGGER_CONSOLE_STREAM_H_

#include "framework/logger/Stream.h"

namespace framework
{
    namespace logger
    {
        /// 写入标准输出
        class ConsoleStream
            : public Stream
        {
        public:
            ConsoleStream();

            virtual ~ConsoleStream();

        private:
            virtual void write( 
                buffer_t const * logmsgs, 
                size_t len );

            virtual void load_config(
                framework::configure::ConfigModule & cm);

        private:
#ifdef BOOST_WINDOWS_API
            HANDLE handle_;
#else
            int fd_;
#endif
            bool color_;
        };
        
        LOG_REG_STREAM_TYPE(console, ConsoleStream)

    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_CONSOLE_STREAM_H_
