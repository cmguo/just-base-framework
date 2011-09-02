#ifndef _FRAMEWORK_LOGGER_LOGGERMANAGER_H_
#define _FRAMEWORK_LOGGER_LOGGERMANAGER_H_

#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>

namespace framework
{
    namespace configure
    {
        class ConfigModule;
        class Config;
    }

    namespace logger
    {
        struct Logger;
        struct ILoggerStream;

        /// 缓冲进程线程ID
        struct PidBuffer 
        {
            PidBuffer( bool havepid, bool havetid )
            {
                init_data( havepid, havetid );
            }

            /// 初始化缓冲区
            void init_data( bool havepid, bool havetid );

            char pid_buffer_[32];
        };

        /// 配置输出进程、线程ID
        struct LoggerDefine
        {
            bool log_pid;
            bool log_tid;

            boost::thread_specific_ptr<PidBuffer> str;
            size_t str_len;

            LoggerDefine()
                : log_pid( false )
                , log_tid( false )
                , str_len( 0 ) { }

            char * get()
            {
                if ( !str.get() )
                {
                    str.reset( new PidBuffer( log_pid, log_tid ) );
                    str_len = strlen( str.get()->pid_buffer_ );
                }

                return str.get()->pid_buffer_;
            }
        };

        /// 模块定义
        struct LogModule
        {
            LogModule * next;       /// 下一个模块
            Logger * logger_;       /// 引用的日志模块
            char const * name;      /// 当前模块的名称
            size_t level;           /// 模块等级，用于一级过滤

            LogModule(
                Logger * logger
                , char const * mname = NULL
                , size_t mlevel = 0 )
                : next( NULL )
                , logger_(logger)
                , name( mname )
                , level( mlevel )
            {
            }
        };

        /// 日志管理类
        class LoggerManager
        {
        public:
            /// 构造、析构
            LoggerManager();
            ~LoggerManager();

			/*FRIEND_TEST(framework_logger, stream_add);
			FRIEND_TEST(framework_logger, module_register);
			FRIEND_TEST(framework_logger, log_create);*/


            /// 从配置文件读取每个输出流，创建输出流队列
            boost::system::error_code load_config( 
                framework::configure::Config & conf );

            /// 注册模块
            LogModule & register_module(
                char const * name, 
                size_t level = 6,// the default level
                char const * log_name = NULL);

            ///.添加一个流
            ILoggerStream & add_stream( ILoggerStream * ls );

            ///.添加一个流到指定日志
            bool add_stream_to( ILoggerStream * ls, std::string const & logname  );

            /// 删除一个流
            bool del_stream( ILoggerStream * ls );

            /// 创建一个日志模块
            Logger & createLogger( std::string const & logName );

            /// 查找流
            ILoggerStream & getStream( std::string const & streamName );

            /// 查找日志模块
            Logger & getLogger( std::string const & logName = "" );

            /// 查找模块
            LogModule & getModule( std::string const & moduleName );

            /// 获取当前版本号
            size_t getVer() const { return m_sec_ver_; }

            /// 获取进程线程定义
            LoggerDefine & getLoggerDefine() { return *m_logdefine_; }

            /// 段输出
            void log_sec_beg() {++m_sec_ver_;}
            void log_sec_end() {++m_sec_ver_;}

        private:
            /// 插入输出流链表
            ILoggerStream * insertStreamList( ILoggerStream * ls );

            /// 插入日志链表
            Logger * insertLoggerList( Logger * lg );

            /// 插入模块链表
            LogModule * insertModuleList( LogModule * lm );

            /// 初始化输出流链表
            void initStreamList( framework::configure::Config & conf );

            /// 初始化日志链表
            void initLoggerList( framework::configure::Config & conf );

            /// 初始化模块结构
            void initModuleTree( framework::configure::Config & conf );

        private:
            ILoggerStream * m_log_streams_;     /// 输出流链表
            Logger * m_loggers_;                /// 日志链表
            LogModule * m_logmodules_;          /// 模块链表
            Logger * m_default_logger_;         /// 默认日志

            /// 线程锁
            boost::mutex m_stream_mutex_;
            boost::mutex m_logger_mutex_;
            boost::mutex m_module_mutex_;

            size_t m_sec_ver_;
            LoggerDefine * m_logdefine_;

            std::map< std::string , std::map< std::string , std::string > > m_module_info_;

            /// 读写锁
            // RWLock * m_rw_lock_;
        };

        extern LoggerManager & glog_mgr;

        LoggerManager & global_logger_mgr();

        inline static LoggerManager & _slog_mgr(...)
        {
            return global_logger_mgr();
        }

    } // namespace logger
} // namespace framework

#define FRAMEWORK_LOGGER_DECLARE_LOGGER(name) \
    inline static framework::logger::LoggerManager & _slog_mgr() \
{ \
    static framework::logger::LoggerManager logmgr; \
    return logmgr; \
}

/// 注册一个模块，默认等级为6，插入到默认日志模块
#define FRAMEWORK_LOGGER_DECLARE_MODULE(name) \
    inline static framework::logger::LogModule const & _slogm() \
{ \
    using framework::logger::_slog_mgr; \
    static framework::logger::LogModule const & module = \
    _slog_mgr().register_module(name); \
    return module; \
}

/// 注册一个模块，指定模块等级，插入到默认日志模块
#define FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL(name, lvl) \
    inline static framework::logger::LogModule const & _slogm() \
{ \
    using framework::logger::_slog_mgr; \
    static framework::logger::LogModule const & module = \
    _slog_mgr().register_module(name, lvl); \
    return module; \
}

/// 注册一个模块，指定模块等级，插入指定日志模块
#define FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL_LOGGER(name, lvl, mod_name) \
    inline static framework::logger::LogModule const & _slogm() \
{ \
    using framework::logger::_slog_mgr; \
    static framework::logger::LogModule const & module = \
    _slog_mgr().register_module(name, lvl, mod_name); \
    return module; \
}

#define FRAMEWORK_LOGGER_DECLARE_MODULE_USE_BASE(base) \
    using base::_slogm

#endif // _FRAMEWORK_LOGGER_LOGGERMANAGER_H_


/*LoggerManager.h接口测试代码*/
#ifdef INCLUDE_TEST

#ifndef _FRAMEWORK_LOGGER_LOGGERMANAGER_TEST_H_
#define _FRAMEWORK_LOGGER_LOGGERMANAGER_TEST_H_

#ifndef FRAMEWORK_LOGGER_DECLARE 
#define FRAMEWORK_LOGGER_DECLARE
MODULE_DECLARE("framework_logger", "darrenhe", "leochen");
#endif


#include "framework/logger/Logger.h"
#include "framework/configure/Config.h"
#include "framework/system/LogicError.h"
#include "framework/logger/LoggerStdStream.h"

using namespace framework::logger;

/*LoggerManager::add_stream()测试*/
TEST(framework_logger, stream_add)
{
	/*虚构具有默认级别的流，验证是否能添加成功*/
	LoggerManager log_mgr;
    
	LoggerStdStream *lgss = new LoggerStdStream();
    ILoggerStream * ils = lgss;
	log_mgr.add_stream(ils);
	EXPECT_TRUE(log_mgr.m_log_streams_ == ils);

	/*添加流，验证返回值是否正确*/

	LoggerStdStream *lgss1 = new LoggerStdStream();
	ILoggerStream * ils1 = lgss1;
	EXPECT_TRUE(&(log_mgr.add_stream(ils1)) == ils1);
	/*以默认级别排序的流验证*/
	EXPECT_TRUE(log_mgr.m_log_streams_ == ils1);
	EXPECT_TRUE(log_mgr.m_log_streams_->next == ils);


	/*自定义某级别的流，验证添加流后流链表顺序正确性*/
	LoggerStdStream *lgss2 = new LoggerStdStream();
	ILoggerStream * ils2 = lgss2;
	ils2->log_lvl = 3;
	LoggerStdStream *lgss3 = new LoggerStdStream();
	ILoggerStream * ils3 = lgss3;
	ils3->log_lvl = 2;
    /*流链表是无序的，最新插入的流在头部*/
	log_mgr.add_stream(ils2);
	log_mgr.add_stream(ils3);
	EXPECT_TRUE(log_mgr.m_log_streams_ == ils3);
	EXPECT_TRUE(log_mgr.m_log_streams_->next == ils2);
	EXPECT_TRUE(log_mgr.m_log_streams_->next->next == ils1);
	EXPECT_TRUE(log_mgr.m_log_streams_->next->next->next == ils);

}

/*LoggerManager::load_config()接口测试*/
TEST(framework_logger, config_load)
{
	/*加载配置文件是否成功测试*/
	LoggerManager lg_mgr;
	framework::configure::Config conf("test.conf");

	EXPECT_EQ(framework::system::logic_error::errors::succeed,
		lg_mgr.load_config(conf));
}

/*LoggerManager::register_module()接口测试*/
TEST(framework_logger, module_register)
{
	/*构造默认logger以及log链表*/
	LoggerManager log_mgr;
    log_mgr.m_default_logger_ = new Logger(log_mgr, "default_log");
	Logger *lg1 = new Logger(log_mgr, "log_test");
	Logger *lg2 = new Logger(log_mgr, "log_test1");
    log_mgr.m_loggers_ = lg1;
	lg1->next = lg2;

	/*模块链表初始时为空的情况*/	
	log_mgr.register_module("test", 4, "log_test");

	EXPECT_TRUE(log_mgr.m_logmodules_ != NULL);
	EXPECT_TRUE(log_mgr.m_logmodules_->name == "test");
	EXPECT_TRUE(log_mgr.m_logmodules_->level == 4);	
	EXPECT_STREQ(log_mgr.m_logmodules_->logger_->m_log_name_.c_str(),"log_test");

	/*模块链表初始时非空的情况*/
	log_mgr.register_module("test1", 5, "log_test1");

	EXPECT_TRUE(log_mgr.m_logmodules_->name == "test1");
	EXPECT_TRUE(log_mgr.m_logmodules_->level == 5);
	EXPECT_STREQ(log_mgr.m_logmodules_->logger_->m_log_name_ .c_str(),"log_test1");

	/*模块使用默认参数情况*/
	log_mgr.register_module("default");

	EXPECT_TRUE(log_mgr.m_logmodules_->name == "default");
	EXPECT_TRUE(log_mgr.m_logmodules_->level == 6);
	EXPECT_STREQ(log_mgr.m_logmodules_->logger_->m_log_name_ .c_str(), "default_log");

	/*log链表中不存在log名情况，函数返回值测试*/
	LogModule lm = log_mgr.register_module("test_return", 5, "log_test_return");

	EXPECT_TRUE(lm.name == "test_return");
	EXPECT_TRUE(lm.level == 5);
	EXPECT_STREQ(lm.logger_->m_log_name_ .c_str(), "default_log");

	/*默认参数时函数返回值测试*/
	LogModule lm1 = log_mgr.register_module("test_return1");

	EXPECT_TRUE(lm1.name == "test_return1");
	EXPECT_TRUE(lm1.level == 6);
	EXPECT_STREQ(lm1.logger_->m_log_name_ .c_str(), "default_log");

}

/*LoggerManager::del_stream()测试*/
TEST(framework_logger, stream_del)
{
	/*删除的流不存在时的情况测试*/
	LoggerManager log_mgr;
	LoggerStdStream *lgss = new LoggerStdStream();
	ILoggerStream * il = lgss;
	bool b = false;
	if (log_mgr.del_stream(il))
	{
		b = true;
	}
	EXPECT_FALSE(b);

	/*删除流存在的情况测试*/
	b = false;
	log_mgr.add_stream(il);
	if (log_mgr.del_stream(il))
	{
		b = true;
	}
	EXPECT_TRUE(b);
	
}


/*LoggerManager::add_stream_to()接口测试*/
/*log是链表中不存在的会报错，流需要先添加到链表再添加到log*/
/*添加相同流到链表会报错*/
TEST(framework_logger, stream_add_to)
{
	/*创建log链表*/
	LoggerManager lg_mgr;
	Logger &lg = lg_mgr.createLogger("log_test");
	lg_mgr.createLogger("log_test1");

	/*创建流*/
	LoggerStdStream *lgss = new LoggerStdStream();
	ILoggerStream * ils = lgss;

	/*加入的log在Log链表上不存在的情况测试*/
    /*ils流已经插入到流链表中*/
	EXPECT_FALSE(lg_mgr.add_stream_to(ils, "test"));

	/* 正常添加流操作测试*/	
	LoggerStdStream *lgss1 = new LoggerStdStream();
	ILoggerStream * ils1 = lgss1;
	EXPECT_TRUE(lg_mgr.add_stream_to(ils1, "log_test"));

	/* 流添加容错测试*/
	EXPECT_TRUE(lg_mgr.add_stream_to(ils1, "log_test"));
	EXPECT_TRUE(lg_mgr.add_stream_to(ils1, "log_test1"));
}


/*LoggerManager::createLogger()接口测试*/
TEST(framework_logger, log_create)
{
	/*level为默认值情况测试*/
	LoggerManager lg_mgr;
	Logger& lg = lg_mgr.createLogger("log");
	bool is_null;
	if (&lg == NULL)
	{
		is_null = true;
	}
	else
		is_null = false;
	EXPECT_FALSE(is_null);
	if(!is_null)
	{
		EXPECT_STREQ("log", lg.m_log_name_.c_str());
	}

	
	Logger& lg1 = lg_mgr.createLogger("log1");
	bool is_null1;
	if (&lg1 == NULL)
	{
		is_null1 = true;
	}
	else
		is_null1 = false;
	EXPECT_FALSE(is_null1);
	if(!is_null1)
	{
		EXPECT_STREQ("log1", lg1.m_log_name_.c_str());
	}

}

/*LoggerManager::getStream()接口测试*/
TEST(framework_logger, stream_get)
{
	LoggerManager  lg_mgr;
	LoggerStdStream *stds = new LoggerStdStream();
	stds->stream_name_ = "std_stream";
	stds->stream_type_ = "STD";
	ILoggerStream *ils = (ILoggerStream *) stds;

	/*流链表不存在获取流的情况测试*/
	ILoggerStream & ils1 = lg_mgr.getStream("std_stream");
	bool is_null;
	if (&ils1 == NULL)
	{
		is_null = true;
	}
	else
		is_null = false;
	EXPECT_TRUE(is_null);

	/*流链表中存在获取流的情况测试*/
	lg_mgr.add_stream(ils);
	ILoggerStream & ils2 = lg_mgr.getStream("std_stream");
	bool is_null1;
	if (&ils2 == NULL)
	{
		is_null1 = true;
	}
	else
		is_null1 = false;
	EXPECT_FALSE(is_null1);
	EXPECT_STREQ("std_stream", ils2.stream_name_.c_str());

}


/*LoggerManager::getLogger()接口测试*/
TEST(framework_logger, logger_get)
{
	/*无log名称参数,Log链表不为空的情况测试*/
	LoggerManager lg_mgr;
	lg_mgr.createLogger("log");	
	Logger& lg = lg_mgr.getLogger();
	bool is_null;
	if (&lg == NULL)
	{
		is_null = true;
	}
	else
		is_null = false;
	EXPECT_TRUE(is_null);

	/*有log名称参数情况测试*/
	Logger& lg1 = lg_mgr.getLogger("log");
	EXPECT_STREQ("log", lg1.m_log_name_.c_str());

}

/*LoggerManager::getModule()接口测试*/
TEST(framework_logger, module_get)
{
	/*module不存在情况测试*/
	LoggerManager lg_mgr;
	LogModule & lm = lg_mgr.getModule("module");
	bool is_null;
	if (&lm == NULL)
	{
		is_null = true;
	}
	else
		is_null = false;
	EXPECT_TRUE(is_null);

	/*module存在情况测试*/
	Logger * lg = new Logger(lg_mgr, "module_log");
	LogModule * lm1 = new LogModule(lg, "module");
	lg_mgr.m_logmodules_ = lm1;
    LogModule & lm2 = lg_mgr.getModule("module");

	if (&lm2 == NULL)
	{
		is_null = true;
	}
	else
		is_null = false;
	EXPECT_FALSE(is_null);
}


#endif // _FRAMEWORK_LOGGER_LOGGERMANAGER_TEST_H_

#endif // INCLUDE_TEST

