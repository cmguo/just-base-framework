#ifndef _FRAMEWORK_LOGGER_LOGGERFILESTREAM_H_
#define _FRAMEWORK_LOGGER_LOGGERFILESTREAM_H_

#include "framework/logger/LoggerStream.h"

namespace framework
{
    namespace logger
    {
        /// 写入文件
        class LoggerFileStream : public ILoggerStream
        {
        public:
            //FRIEND_TEST(framework_logger, file_backup);
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



/*LoggerFileStream提供私有接口测试代码*/
#ifdef INCLUDE_TEST

#ifndef _FRAMEWORK_LOGGER_LOGGERFILESTREAM_TEST_H_
#define _FRAMEWORK_LOGGER_LOGGERFILESTREAM_TEST_H_


#ifndef FRAMEWORK_LOGGER_DECLARE 
#define FRAMEWORK_LOGGER_DECLARE
MODULE_DECLARE("framework_logger", "darrenhe", "leochen");
#endif //FRAMEWORK_LOGGER_DECLARE



/*LoggerFileStream::check_file()接口测试*/
//TEST(framework_logger, file_check)
//{
//
//}

/*LoggerFileStream::backup_file()接口测试*/
//using namespace framework::logger;
//TEST(framework_logger, file_backup)
//{
//
//#ifdef BOOST_WINDOWS_API
//	/*创建日志文件*/
//	LoggerFileStream lfs("log_name");
//	FILE * fd;
//	errno_t err = fopen_s(&fd, "log_name", "w");
//	EXPECT_FALSE(err);
//    fclose(fd);
//
//	char buf[40];
//	ILoggerStream::time_str_now(buf, sizeof(buf), ".%Y-%m-%d %HH%MM%SS");
//	lfs.backup_file("log_name");
//
//	err = fopen_s(&fd, ("log_bak/" +lfs.m_log_file_ + buf).c_str(), "r");
//	EXPECT_FALSE(err);
//	fclose(fd);
//
//	remove("log_name");
//	remove(("log_bak/" +lfs.m_log_file_ + buf).c_str());
//
//#else
//    LoggerFileStream lfs("log_name");
//	int fd;
//
//	fd = ::open("log_name", O_RDONLY | O_CREAT);
//	EXPECT_FALSE(-1 == fd);
//	char buf[40];
//	ILoggerStream::time_str_now(buf, sizeof(buf), ".%Y-%m-%d %HH%MM%SS");
//	::close(fd);
//	lfs.backup_file("log_name");
//	fd = ::open(("log_bak/" +lfs.m_log_file_ + buf).c_str(), O_RDONLY);
//	EXPECT_FALSE(-1 == fd);
//	::close(fd);
//	::unlink("log_name");
//	::unlink(("log_bak/" +lfs.m_log_file_ + buf).c_str());
//	
//#endif	
//}


#endif //_FRAMEWORK_LOGGER_LOGGERFILESTREAM_TEST_H_
#endif //INCLUDE_TEST

