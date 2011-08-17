#ifndef _FRAMEWORK_LOGGER_LOGGERFILESTREAM_H_
#define _FRAMEWORK_LOGGER_LOGGERFILESTREAM_H_

#include "framework/logger/LoggerStream.h"

namespace framework
{
    namespace logger
    {
        /// Ð´ÈëÎÄ¼þ
        class LoggerFileStream : public ILoggerStream
        {
        public:
            LoggerFileStream( 
                std::string const & file, 
                size_t size = 0, 
                bool app = false, 
                bool day = false, 
                bool roll = false );

            virtual ~LoggerFileStream() {}

            virtual void write( 
                muti_buffer_t const * logmsgs, 
                size_t len );

        private:
            bool check_file();
            bool backup_file( char const * filename );
            void at_new_time();

        private:
#ifdef BOOST_WINDOWS_API
            FILE * fd_;
#else
            int fd_;
#endif
            bool m_log_app_;
            bool m_log_day_;
            bool m_log_roll_;
            size_t m_log_size_;
            size_t mid_night_;
            std::string m_log_file_;
        };

    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_LOGGERFILESTREAM_H_
