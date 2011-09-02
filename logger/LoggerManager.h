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

        /// ��������߳�ID
        struct PidBuffer 
        {
            PidBuffer( bool havepid, bool havetid )
            {
                init_data( havepid, havetid );
            }

            /// ��ʼ��������
            void init_data( bool havepid, bool havetid );

            char pid_buffer_[32];
        };

        /// ����������̡��߳�ID
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

        /// ģ�鶨��
        struct LogModule
        {
            LogModule * next;       /// ��һ��ģ��
            Logger * logger_;       /// ���õ���־ģ��
            char const * name;      /// ��ǰģ�������
            size_t level;           /// ģ��ȼ�������һ������

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

        /// ��־������
        class LoggerManager
        {
        public:
            /// ���졢����
            LoggerManager();
            ~LoggerManager();

			/*FRIEND_TEST(framework_logger, stream_add);
			FRIEND_TEST(framework_logger, module_register);
			FRIEND_TEST(framework_logger, log_create);*/


            /// �������ļ���ȡÿ����������������������
            boost::system::error_code load_config( 
                framework::configure::Config & conf );

            /// ע��ģ��
            LogModule & register_module(
                char const * name, 
                size_t level = 6,// the default level
                char const * log_name = NULL);

            ///.���һ����
            ILoggerStream & add_stream( ILoggerStream * ls );

            ///.���һ������ָ����־
            bool add_stream_to( ILoggerStream * ls, std::string const & logname  );

            /// ɾ��һ����
            bool del_stream( ILoggerStream * ls );

            /// ����һ����־ģ��
            Logger & createLogger( std::string const & logName );

            /// ������
            ILoggerStream & getStream( std::string const & streamName );

            /// ������־ģ��
            Logger & getLogger( std::string const & logName = "" );

            /// ����ģ��
            LogModule & getModule( std::string const & moduleName );

            /// ��ȡ��ǰ�汾��
            size_t getVer() const { return m_sec_ver_; }

            /// ��ȡ�����̶߳���
            LoggerDefine & getLoggerDefine() { return *m_logdefine_; }

            /// �����
            void log_sec_beg() {++m_sec_ver_;}
            void log_sec_end() {++m_sec_ver_;}

        private:
            /// �������������
            ILoggerStream * insertStreamList( ILoggerStream * ls );

            /// ������־����
            Logger * insertLoggerList( Logger * lg );

            /// ����ģ������
            LogModule * insertModuleList( LogModule * lm );

            /// ��ʼ�����������
            void initStreamList( framework::configure::Config & conf );

            /// ��ʼ����־����
            void initLoggerList( framework::configure::Config & conf );

            /// ��ʼ��ģ��ṹ
            void initModuleTree( framework::configure::Config & conf );

        private:
            ILoggerStream * m_log_streams_;     /// ���������
            Logger * m_loggers_;                /// ��־����
            LogModule * m_logmodules_;          /// ģ������
            Logger * m_default_logger_;         /// Ĭ����־

            /// �߳���
            boost::mutex m_stream_mutex_;
            boost::mutex m_logger_mutex_;
            boost::mutex m_module_mutex_;

            size_t m_sec_ver_;
            LoggerDefine * m_logdefine_;

            std::map< std::string , std::map< std::string , std::string > > m_module_info_;

            /// ��д��
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

/// ע��һ��ģ�飬Ĭ�ϵȼ�Ϊ6�����뵽Ĭ����־ģ��
#define FRAMEWORK_LOGGER_DECLARE_MODULE(name) \
    inline static framework::logger::LogModule const & _slogm() \
{ \
    using framework::logger::_slog_mgr; \
    static framework::logger::LogModule const & module = \
    _slog_mgr().register_module(name); \
    return module; \
}

/// ע��һ��ģ�飬ָ��ģ��ȼ������뵽Ĭ����־ģ��
#define FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL(name, lvl) \
    inline static framework::logger::LogModule const & _slogm() \
{ \
    using framework::logger::_slog_mgr; \
    static framework::logger::LogModule const & module = \
    _slog_mgr().register_module(name, lvl); \
    return module; \
}

/// ע��һ��ģ�飬ָ��ģ��ȼ�������ָ����־ģ��
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


/*LoggerManager.h�ӿڲ��Դ���*/
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

/*LoggerManager::add_stream()����*/
TEST(framework_logger, stream_add)
{
	/*�鹹����Ĭ�ϼ����������֤�Ƿ�����ӳɹ�*/
	LoggerManager log_mgr;
    
	LoggerStdStream *lgss = new LoggerStdStream();
    ILoggerStream * ils = lgss;
	log_mgr.add_stream(ils);
	EXPECT_TRUE(log_mgr.m_log_streams_ == ils);

	/*���������֤����ֵ�Ƿ���ȷ*/

	LoggerStdStream *lgss1 = new LoggerStdStream();
	ILoggerStream * ils1 = lgss1;
	EXPECT_TRUE(&(log_mgr.add_stream(ils1)) == ils1);
	/*��Ĭ�ϼ������������֤*/
	EXPECT_TRUE(log_mgr.m_log_streams_ == ils1);
	EXPECT_TRUE(log_mgr.m_log_streams_->next == ils);


	/*�Զ���ĳ�����������֤�������������˳����ȷ��*/
	LoggerStdStream *lgss2 = new LoggerStdStream();
	ILoggerStream * ils2 = lgss2;
	ils2->log_lvl = 3;
	LoggerStdStream *lgss3 = new LoggerStdStream();
	ILoggerStream * ils3 = lgss3;
	ils3->log_lvl = 2;
    /*������������ģ����²��������ͷ��*/
	log_mgr.add_stream(ils2);
	log_mgr.add_stream(ils3);
	EXPECT_TRUE(log_mgr.m_log_streams_ == ils3);
	EXPECT_TRUE(log_mgr.m_log_streams_->next == ils2);
	EXPECT_TRUE(log_mgr.m_log_streams_->next->next == ils1);
	EXPECT_TRUE(log_mgr.m_log_streams_->next->next->next == ils);

}

/*LoggerManager::load_config()�ӿڲ���*/
TEST(framework_logger, config_load)
{
	/*���������ļ��Ƿ�ɹ�����*/
	LoggerManager lg_mgr;
	framework::configure::Config conf("test.conf");

	EXPECT_EQ(framework::system::logic_error::errors::succeed,
		lg_mgr.load_config(conf));
}

/*LoggerManager::register_module()�ӿڲ���*/
TEST(framework_logger, module_register)
{
	/*����Ĭ��logger�Լ�log����*/
	LoggerManager log_mgr;
    log_mgr.m_default_logger_ = new Logger(log_mgr, "default_log");
	Logger *lg1 = new Logger(log_mgr, "log_test");
	Logger *lg2 = new Logger(log_mgr, "log_test1");
    log_mgr.m_loggers_ = lg1;
	lg1->next = lg2;

	/*ģ�������ʼʱΪ�յ����*/	
	log_mgr.register_module("test", 4, "log_test");

	EXPECT_TRUE(log_mgr.m_logmodules_ != NULL);
	EXPECT_TRUE(log_mgr.m_logmodules_->name == "test");
	EXPECT_TRUE(log_mgr.m_logmodules_->level == 4);	
	EXPECT_STREQ(log_mgr.m_logmodules_->logger_->m_log_name_.c_str(),"log_test");

	/*ģ�������ʼʱ�ǿյ����*/
	log_mgr.register_module("test1", 5, "log_test1");

	EXPECT_TRUE(log_mgr.m_logmodules_->name == "test1");
	EXPECT_TRUE(log_mgr.m_logmodules_->level == 5);
	EXPECT_STREQ(log_mgr.m_logmodules_->logger_->m_log_name_ .c_str(),"log_test1");

	/*ģ��ʹ��Ĭ�ϲ������*/
	log_mgr.register_module("default");

	EXPECT_TRUE(log_mgr.m_logmodules_->name == "default");
	EXPECT_TRUE(log_mgr.m_logmodules_->level == 6);
	EXPECT_STREQ(log_mgr.m_logmodules_->logger_->m_log_name_ .c_str(), "default_log");

	/*log�����в�����log���������������ֵ����*/
	LogModule lm = log_mgr.register_module("test_return", 5, "log_test_return");

	EXPECT_TRUE(lm.name == "test_return");
	EXPECT_TRUE(lm.level == 5);
	EXPECT_STREQ(lm.logger_->m_log_name_ .c_str(), "default_log");

	/*Ĭ�ϲ���ʱ��������ֵ����*/
	LogModule lm1 = log_mgr.register_module("test_return1");

	EXPECT_TRUE(lm1.name == "test_return1");
	EXPECT_TRUE(lm1.level == 6);
	EXPECT_STREQ(lm1.logger_->m_log_name_ .c_str(), "default_log");

}

/*LoggerManager::del_stream()����*/
TEST(framework_logger, stream_del)
{
	/*ɾ������������ʱ���������*/
	LoggerManager log_mgr;
	LoggerStdStream *lgss = new LoggerStdStream();
	ILoggerStream * il = lgss;
	bool b = false;
	if (log_mgr.del_stream(il))
	{
		b = true;
	}
	EXPECT_FALSE(b);

	/*ɾ�������ڵ��������*/
	b = false;
	log_mgr.add_stream(il);
	if (log_mgr.del_stream(il))
	{
		b = true;
	}
	EXPECT_TRUE(b);
	
}


/*LoggerManager::add_stream_to()�ӿڲ���*/
/*log�������в����ڵĻᱨ������Ҫ����ӵ���������ӵ�log*/
/*�����ͬ��������ᱨ��*/
TEST(framework_logger, stream_add_to)
{
	/*����log����*/
	LoggerManager lg_mgr;
	Logger &lg = lg_mgr.createLogger("log_test");
	lg_mgr.createLogger("log_test1");

	/*������*/
	LoggerStdStream *lgss = new LoggerStdStream();
	ILoggerStream * ils = lgss;

	/*�����log��Log�����ϲ����ڵ��������*/
    /*ils���Ѿ����뵽��������*/
	EXPECT_FALSE(lg_mgr.add_stream_to(ils, "test"));

	/* �����������������*/	
	LoggerStdStream *lgss1 = new LoggerStdStream();
	ILoggerStream * ils1 = lgss1;
	EXPECT_TRUE(lg_mgr.add_stream_to(ils1, "log_test"));

	/* ������ݴ����*/
	EXPECT_TRUE(lg_mgr.add_stream_to(ils1, "log_test"));
	EXPECT_TRUE(lg_mgr.add_stream_to(ils1, "log_test1"));
}


/*LoggerManager::createLogger()�ӿڲ���*/
TEST(framework_logger, log_create)
{
	/*levelΪĬ��ֵ�������*/
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

/*LoggerManager::getStream()�ӿڲ���*/
TEST(framework_logger, stream_get)
{
	LoggerManager  lg_mgr;
	LoggerStdStream *stds = new LoggerStdStream();
	stds->stream_name_ = "std_stream";
	stds->stream_type_ = "STD";
	ILoggerStream *ils = (ILoggerStream *) stds;

	/*���������ڻ�ȡ�����������*/
	ILoggerStream & ils1 = lg_mgr.getStream("std_stream");
	bool is_null;
	if (&ils1 == NULL)
	{
		is_null = true;
	}
	else
		is_null = false;
	EXPECT_TRUE(is_null);

	/*�������д��ڻ�ȡ�����������*/
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


/*LoggerManager::getLogger()�ӿڲ���*/
TEST(framework_logger, logger_get)
{
	/*��log���Ʋ���,Log����Ϊ�յ��������*/
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

	/*��log���Ʋ����������*/
	Logger& lg1 = lg_mgr.getLogger("log");
	EXPECT_STREQ("log", lg1.m_log_name_.c_str());

}

/*LoggerManager::getModule()�ӿڲ���*/
TEST(framework_logger, module_get)
{
	/*module�������������*/
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

	/*module�����������*/
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

