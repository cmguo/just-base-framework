// FileStream.h

#ifndef _FRAMEWORK_LOGGER_FILE_STREAM_H_
#define _FRAMEWORK_LOGGER_FILE_STREAM_H_

#include "framework/logger/Stream.h"
#include "framework/logger/Time.h"

#include <boost/thread/mutex.hpp>

namespace framework
{
    namespace logger
    {
        /// Ð´ÈëÎÄ¼þ
        class FileStream
            : public Stream
        {
        public:
            FileStream();

            virtual ~FileStream();

        private:
            virtual void write( 
                buffer_t const * logmsgs, 
                size_t len);

            virtual void load_config(
                framework::configure::ConfigModule & cm);

        private:
            bool open();

            bool is_open() const;

            bool close();

            bool reopen();

            bool seek(
                bool beg_or_end);

            size_t position();

            bool check_file();

            bool backup_file();

        private:
            Time time_;
            boost::mutex mutex_;
#ifdef BOOST_WINDOWS_API
            HANDLE handle_;
#else
            int fd_;
#endif
            bool app_;
            bool daily_;
            bool roll_;
            size_t size_;
            std::string file_;
        };

        LOG_REG_STREAM_TYPE(file, FileStream)

    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_LOGGERFILESTREAM_H_
