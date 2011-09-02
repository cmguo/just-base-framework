// Logger.h

#ifndef _FRAMEWORK_LOGGER_LOGGER_H_
#define _FRAMEWORK_LOGGER_LOGGER_H_

#include "framework/logger/LoggerRecord.h"
#include "framework/logger/LoggerManager.h"

namespace framework
{
    namespace logger
    {
        /// ��־����ȼ�����
        enum LogLevel
        {
            kLevelError     = 0,    // ��Ϣ����Ķ���
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
                kLevelError     = 0,    // ��Ϣ����Ķ���
                kLevelAlarm,
                kLevelEvent,
                kLevelInfor,
                kLevelDebug,
                kLevelDebug1,                kLevelDebug2,                kLevelNone            };

        public:
            struct LoggerStreams
            {
                LoggerStreams( ILoggerStream * stream ) : stream_( stream ), is_del_( false ), next( NULL ) {}
                ILoggerStream * stream_;            /// ������ָ��
                bool is_del_;                     /// �Ƿ�ɾ��
                LoggerStreams * next;               /// ��һ���ڵ�
            };

        public:
            Logger * next;                          /// ��һ����־
            std::string m_log_name_;                /// ��־����
            size_t mid_night_;
            size_t time_ver_;
            char time_str_[40];                     /// ���ڴ�
            size_t m_max_level_;                    /// ��ǰָ���������ȼ������ڶ�������
            LoggerStreams * mp_log_streams_;        /// ӵ�е���
            LoggerManager & m_log_mgr_;             /// �����������
            boost::mutex streams_mutex_;            /// ��

        public:
            Logger( LoggerManager & logmgr, std::string const & name );
            ~Logger();

            /// ����һ����
            void insertStream( ILoggerStream * ls );

            /// ɾ��һ����
            void delStream( ILoggerStream * ls );

        public:
            /// �����־
            void printLog(
                LogModule const & module, 
                size_t level, 
                LoggerRecord const & record);

            /// �ַ�����ӡ
            void printString(
                LogModule const & module,
                size_t level, 
                char const * txt, 
                size_t len = 0);

            /// ��ӡʮ������
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
                    || ( level > module.level ) /// ����ģ������ȼ�����
                    || ( level > module.logger_->m_max_level_ ) )/// ���������ȼ�����
                    return;

                // ���ݵȼ�����õ������С������д�ӡ
                module.logger_->printLog( module, level, record);
            }

            // ���һ�����ýṹ����Ϣ��ʽ����־��Ϣ
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
                    || ( level > module.level ) /// ����ģ������ȼ�����
                    || ( level > module.logger_->m_max_level_ ) )/// ���������ȼ�����
                    return;
            }

            // ���һ�����ݽϳ�����־��Ϣ�������Ϣ���ݿ��ܳ���1024�ֽڣ�ʹ�ô˽ӿڣ�����ᱻ�м�ض�
            static inline void print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt)
            {
                if ( !( &module ) ) return; /// MultiThread env.
                if ( level >= kLevelNone
                    || level < 0
                    || ( level > module.level ) /// ����ģ������ȼ�����
                    || ( level > module.logger_->m_max_level_ ) )/// ���������ȼ�����
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
                    || ( level > module.level ) /// ����ģ������ȼ�����
                    || ( level > module.logger_->m_max_level_ ) )/// ���������ȼ�����
                    return;
                module.logger_->printString(  module, level, txt, len );
            }

            // ���һ������16������Ϣ��ʽ����־��Ϣ
            static inline void print_hex(
                LogModule const & module, 
                size_t level, 
                unsigned char const * data, 
                size_t len)
            {
                if ( !( &module ) ) return; /// MultiThread env.
                if ( level >= kLevelNone
                    || level < 0
                    || ( level > module.level ) /// ����ģ������ȼ�����
                    || ( level > module.logger_->m_max_level_ ) )/// ���������ȼ�����
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



/*Logger.h�ӿڲ��Դ���*/
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

/*logger::insertStream()�ӿڲ���*/
TEST(framework_logger, insert_streams)
{
	LoggerManager lg_mgr;
	/*��������ȷ�Բ���*/
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

	/*������������־�ȼ����в���*/
	if (lg.mp_log_streams_->stream_ == il1  && lg.mp_log_streams_->next->stream_ == il)
	{
		b = true;
	}
	else
		b = false;
	EXPECT_TRUE(b);
}

/*logger::delStream()�ӿڲ���*/
TEST(framework_logger, delete_stream)
{
	/*��ɾ����ȷ�Բ���*/
	LoggerManager lg_mgr;
	Logger log(lg_mgr, "log");
	LoggerStdStream *lgss = new LoggerStdStream();
	ILoggerStream * il = lgss;
	log.insertStream(il);
	LoggerStdStream *lgss1 = new LoggerStdStream();
	ILoggerStream * il1 = lgss1;
	log.insertStream(il1);

	//ɾ��һ����
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

	//ɾ���ڶ�����
	log.delStream(il);
	if(log.mp_log_streams_ == NULL)
		b = true;
	else
		b = false;
	EXPECT_TRUE(b);

}


/*Logger::printLog()�ӿڲ���*/

/*�Զ���Log��������ͣ��̳и���ILoggerStream*/
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

/*�Զ���LoggerRecord����LoggerRecordSon*/
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

	/*��log�����в�������LogStream���͵���*/
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


/*Logger::printString()�ӿڲ���*/
/*�Զ���string��������ͣ��̳и���ILoggerStream*/
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
    //��log�����в���һ����
	StringStream *ss1 = new StringStream("test_sprintstring1");
	ILoggerStream *il1 = ss1;
	Logger::LoggerStreams *ls = new Logger::LoggerStreams(il1);
	log.mp_log_streams_ = ls;
    //����log�����в���һ����
	StringStream *ss2 = new StringStream("test_sprintstring2");
	ILoggerStream *il2 = ss2;
    log.insertStream(il2);

	LogModule lm(&log, "test");
	log.print_string(lm, 6, "test_sprintstring1");
	EXPECT_TRUE(ss1->m_write_status);
	log.print_string(lm, 6, "test_sprintstring2");
	EXPECT_TRUE(ss2->m_write_status);

}

/*Logger::print_hex()�ӿڲ���*/
/*�Զ���ʮ��������������ͣ��̳и���ILoggerStream*/
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
	//��log�����в���һ����
	HexStream *hs1 = new HexStream();
	ILoggerStream *il1 = hs1;
	Logger::LoggerStreams *ls = new Logger::LoggerStreams(il1);
	log.mp_log_streams_ = ls;
	//����log�����в���һ����
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

