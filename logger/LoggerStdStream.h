#ifndef _FRAMEWORK_LOGGER_LOGGERSTDSTREAM_H_
#define _FRAMEWORK_LOGGER_LOGGERSTDSTREAM_H_

#include "framework/logger/LoggerStream.h"

namespace framework
{
    namespace logger
    {
        /// 写入标准输出
        class LoggerStdStream : public ILoggerStream
        {
        public:
            LoggerStdStream( bool color = false ) 
                : m_log_clr_( color ) 
            {
            }

            virtual void write( 
                muti_buffer_t const * logmsgs, 
                size_t len );

            virtual ~LoggerStdStream() {}

        private:
            bool m_log_clr_;
        };
    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_LOGGERSTDSTREAM_H_
