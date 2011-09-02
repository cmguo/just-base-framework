// Logger.h

#ifndef _FRAMEWORK_LOGGER_LOGGER_H_
#define _FRAMEWORK_LOGGER_LOGGER_H_

#include "framework/logger/LoggerRecord.h"
#include "framework/logger/LoggerManager.h"

namespace framework
{
    namespace logger
    {
        /// 日志错误等级定义
        enum LogLevel
        {
            kLevelError     = 0,    // 消息级别的定义
            kLevelAlarm,
            kLevelEvent,
            kLevelInfor,
            kLevelDebug,
            kLevelDebug1,            kLevelDebug2,            kLevelNone        };

        struct Logger
        {
        public:
            enum
            {
                kLevelError     = 0,    // 消息级别的定义
                kLevelAlarm,
                kLevelEvent,
                kLevelInfor,
                kLevelDebug,
                kLevelDebug1,                kLevelDebug2,                kLevelNone            };

        public:
            struct LoggerStreams
            {
                LoggerStreams( ILoggerStream * stream ) : stream_( stream ), is_del_( false ), next( NULL ) {}
                ILoggerStream * stream_;            /// 引用流指针
                bool is_del_;                     /// 是否被删除
                LoggerStreams * next;               /// 下一个节点
            };

        public:
            Logger * next;                          /// 下一个日志
            std::string m_log_name_;                /// 日志名称
            size_t mid_night_;
            size_t time_ver_;
            char time_str_[40];                     /// 日期串
            size_t m_max_level_;                    /// 当前指向流的最大等级，用于二级过滤
            LoggerStreams * mp_log_streams_;        /// 拥有的流
            LoggerManager & m_log_mgr_;             /// 管理类的引用
            boost::mutex streams_mutex_;            /// 锁

        public:
            Logger( LoggerManager & logmgr, std::string const & name );
            ~Logger();

            /// 插入一个流
            void insertStream( ILoggerStream * ls );

            /// 删除一个流
            void delStream( ILoggerStream * ls );

        public:
            /// 输出日志
            void printLog(
                LogModule const & module, 
                size_t level, 
                LoggerRecord const & record);

            /// 字符串打印
            void printString(
                LogModule const & module,
                size_t level, 
                char const * txt, 
                size_t len = 0);

            /// 打印十六进制
            void printHex(
                LogModule const & module,
                size_t level, 
                unsigned char const * data, 
                size_t len);

            template <typename _Rec>
            static inline void log(
                LogModule const & module, 
                size_t level, 
                _Rec const & record)
            {
                if ( !( &module ) ) return; /// MultiThread env.
                if ( level >= kLevelNone
                    || level < 0
                    || ( level > module.level ) /// 进行模块的最大等级过滤
                    || ( level > module.logger_->m_max_level_ ) )/// 进行流最大等级过滤
                    return;

                // 根据等级排序好的链表从小到大进行打印
                module.logger_->printLog( module, level, record);
            }

            // 输出一条采用结构化信息格式的日志信息
            static inline void print_struct(
                LogModule const & module, 
                size_t level, 
                char const * title, 
                char const * fmt, 
                void const * var)
            {
                if ( !( &module ) ) return; /// MultiThread env.
                if ( level >= kLevelNone
                    || level < 0
                    || ( level > module.level ) /// 进行模块的最大等级过滤
                    || ( level > module.logger_->m_max_level_ ) )/// 进行流最大等级过滤
                    return;
            }

            // 输出一条内容较长的日志信息，如果消息内容可能超过1024字节，使用此接口，否则会被中间截断
            static inline void print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt)
            {
                if ( !( &module ) ) return; /// MultiThread env.
                if ( level >= kLevelNone
                    || level < 0
                    || ( level > module.level ) /// 进行模块的最大等级过滤
                    || ( level > module.logger_->m_max_level_ ) )/// 进行流最大等级过滤
                    return;
                module.logger_->printString( module, level, txt );
            }


            static inline void print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt, 
                size_t len)
            {
                if ( !( &module ) ) return; /// MultiThread env.
                if ( level >= kLevelNone
                    || level < 0
                    || ( level > module.level ) /// 进行模块的最大等级过滤
                    || ( level > module.logger_->m_max_level_ ) )/// 进行流最大等级过滤
                    return;
                module.logger_->printString(  module, level, txt, len );
            }

            // 输出一条采用16进制信息格式的日志信息
            static inline void print_hex(
                LogModule const & module, 
                size_t level, 
                unsigned char const * data, 
                size_t len)
            {
                if ( !( &module ) ) return; /// MultiThread env.
                if ( level >= kLevelNone
                    || level < 0
                    || ( level > module.level ) /// 进行模块的最大等级过滤
                    || ( level > module.logger_->m_max_level_ ) )/// 进行流最大等级过滤
                    return;
                module.logger_->printHex( module, level, data, len );
            }

      };

    } // namespace logger
} // namespace framework

#define LOG_PKT(lvl, name, fmt, dat) \
    framework::logger::Logger::print_struct(_slogm(), lvl, name, fmt, dat)

#define LOG_STR(lvl, str) \
    framework::logger::Logger::print_string(_slogm(), lvl, str)

#define LOG_STR_LEN(lvl, str, len) \
    framework::logger::Logger::print_string(_slogm(), lvl, str, len)

#define LOG_HEX(lvl, dat, len) \
    framework::logger::Logger::print_hex(_slogm(), lvl, dat, len)

#endif // _FRAMEWORK_LOGGER_LOGGER_H_



/*Logger.h接口测试代码*/
#ifdef INCLUDE_TEST

#ifndef _FRAMEWORK_LOGGER_LOGGER_TEST_H_
#define _FRAMEWORK_LOGGER_LOGGER_TEST_H_

#ifndef FRAMEWORK_LOGGER_DECLARE 
#define FRAMEWORK_LOGGER_DECLARE
MODULE_DECLARE("framework_logger", "darrenhe", "leochen");
#endif

#include "framework/logger/LoggerManager.h"
#include "framework/logger/LoggerStream.h"
#include "framework/logger/LoggerStdStream.h"
#include "framework/logger/LoggerRecord.h"
using namespace framework::logger;

/*logger::insertStream()接口测试*/
TEST(framework_logger, insert_streams)
{
	LoggerManager lg_mgr;
	/*流插入正确性测试*/
	std::string log_name = "log";
	Logger lg(lg_mgr, log_name);
	LoggerStdStream *lgss = new LoggerStdStream();
	ILoggerStream * il = lgss;
	il->log_lvl = 2;
	lg.insertStream(il);
	bool b;
	if(lg.mp_log_streams_->stream_ == NULL)
		b = false;
	else
		b = true;
	EXPECT_TRUE(b);

	LoggerStdStream *lgss1 = new LoggerStdStream();
	ILoggerStream * il1 = lgss1;
	il1->log_lvl = 4;
	lg.insertStream(il1);
	if(lg.mp_log_streams_->next == NULL)
		b = false;
	else
		b = true;
	EXPECT_TRUE(b);

	/*插入流按照日志等级排列测试*/
	if (lg.mp_log_streams_->stream_ == il1  && lg.mp_log_streams_->next->stream_ == il)
	{
		b = true;
	}
	else
		b = false;
	EXPECT_TRUE(b);
}

/*logger::delStream()接口测试*/
TEST(framework_logger, delete_stream)
{
	/*流删除正确性测试*/
	LoggerManager lg_mgr;
	Logger log(lg_mgr, "log");
	LoggerStdStream *lgss = new LoggerStdStream();
	ILoggerStream * il = lgss;
	log.insertStream(il);
	LoggerStdStream *lgss1 = new LoggerStdStream();
	ILoggerStream * il1 = lgss1;
	log.insertStream(il1);

	//删除一个流
	log.delStream(il1);
	
	bool b;
	if(log.mp_log_streams_ != NULL)
		b = true;
	else
		b = false;
	EXPECT_TRUE(b);

	if(log.mp_log_streams_->next == NULL)
		b = true;
	else
		b = false;
	EXPECT_TRUE(b);

	//删除第二个流
	log.delStream(il);
	if(log.mp_log_streams_ == NULL)
		b = true;
	else
		b = false;
	EXPECT_TRUE(b);

}


/*Logger::printLog()接口测试*/

/*自定义Log输出流类型，继承父类ILoggerStream*/
class LogStream : public ILoggerStream
{
public:
	LogStream(){}	

	~LogStream(){}

	virtual void write( 
		muti_buffer_t const * logmsgs, 
		size_t len  )
	{
		if ((1 == len || 4 == len) && logmsgs != NULL)
		{
			m_write_status = true;
		}
		else
			m_write_status = false;
	}

	bool m_write_status ;
};

/*自定义LoggerRecord子类LoggerRecordSon*/
class LoggerRecordSon : public LoggerRecord
{						
public:
	LoggerRecordSon():LoggerRecord(format_message, destroy)
	{

	}
	static size_t format_message( LoggerRecord const & rec, char * msg, size_t len )
	{
        return 0;
	}
	static void destroy(LoggerRecord const & rec)
	{
		return;
	}

};

TEST(framework_logger, log_print)
{
	LoggerManager lg_mgr;
	Logger log(lg_mgr, "log");

	/*向log流队列插入两个LogStream类型的流*/
	LogStream *lgs = new LogStream();
	ILoggerStream *ils = lgs;
	log.insertStream(ils);
	LogStream *lgs1 = new LogStream();
	ILoggerStream *ils1 = lgs1;
	log.insertStream(ils1);	

	LoggerRecordSon  *lrs = new LoggerRecordSon();
	LoggerRecord *lr = lrs;
	
	LogModule lm(&log, "test");

	log.printLog(lm, 6, *lr);
	EXPECT_TRUE(lgs->m_write_status);
	EXPECT_TRUE(lgs1->m_write_status);
}


/*Logger::printString()接口测试*/
/*自定义string输出流类型，继承父类ILoggerStream*/
class StringStream : public ILoggerStream
{
public:
	StringStream(std::string const &txt):m_txt_(txt)	
	{ }		

	~StringStream(){}

	virtual void write( 
		muti_buffer_t const * logmsgs, 
		size_t len  )
	{
		if (logmsgs[3].buffer == m_txt_)
		{
			m_write_status = true;
		}
		else
			m_write_status = false;
	}

	bool m_write_status ;
private:
	std::string m_txt_;
};

TEST(framework_logger, string_print)
{
	LoggerManager lg_mgr;
	Logger log(lg_mgr, "string");
    //向log流队列插入一个流
	StringStream *ss1 = new StringStream("test_sprintstring1");
	ILoggerStream *il1 = ss1;
	Logger::LoggerStreams *ls = new Logger::LoggerStreams(il1);
	log.mp_log_streams_ = ls;
    //再向log流队列插入一个流
	StringStream *ss2 = new StringStream("test_sprintstring2");
	ILoggerStream *il2 = ss2;
    log.insertStream(il2);

	LogModule lm(&log, "test");
	log.print_string(lm, 6, "test_sprintstring1");
	EXPECT_TRUE(ss1->m_write_status);
	log.print_string(lm, 6, "test_sprintstring2");
	EXPECT_TRUE(ss2->m_write_status);

}

/*Logger::print_hex()接口测试*/
/*自定义十六进制输出流类型，继承父类ILoggerStream*/
class HexStream : public ILoggerStream
{
public:
	HexStream( ){ }		

	~HexStream( ){ }

	virtual void write( 
		muti_buffer_t const * logmsgs, 
		size_t len  )
	{
		if ((1 == len || 4 == len) && NULL != logmsgs)
		{
			m_write_status = true;
		}
		else
			m_write_status = false;
	}

	bool m_write_status ;

};

TEST(framework_logger, hex_print)
{
	
	LoggerManager lg_mgr;
	Logger log(lg_mgr, "hex");
	//向log流队列插入一个流
	HexStream *hs1 = new HexStream();
	ILoggerStream *il1 = hs1;
	Logger::LoggerStreams *ls = new Logger::LoggerStreams(il1);
	log.mp_log_streams_ = ls;
	//再向log流队列插入一个流
	HexStream *hs2 = new HexStream();
	ILoggerStream *il2 = hs2;
	log.insertStream(il2);

	LogModule lm(&log, "test");
	unsigned char const c = 'a';
	log.print_hex(lm, 6, &c, 20);
	EXPECT_TRUE(hs1->m_write_status);
	EXPECT_TRUE(hs2->m_write_status);

}

#endif //_FRAMEWORK_LOGGER_LOGGER_TEST_H_
#endif //INCLUDE_TEST

