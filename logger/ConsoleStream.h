// ConsoleStream.h

#ifndef _FRAMEWORK_LOGGER_CONSOLE_STREAM_H_
#define _FRAMEWORK_LOGGER_CONSOLE_STREAM_H_

#include "framework/logger/Stream.h"
#include "framework/filesystem/File.h"

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
            virtual void load_config(
                framework::configure::ConfigModule & cm);

            virtual void write(
                buffers_t const & buffers);

        private:
            framework::filesystem::File file_;
            bool color_;
        };
        
        LOG_REG_STREAM_TYPE(console, ConsoleStream)

    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_CONSOLE_STREAM_H_
