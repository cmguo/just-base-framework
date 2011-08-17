#ifndef _FRAMEWORK_LOGGER_LOGGERMANAGER_H_
#define _FRAMEWORK_LOGGER_LOGGERMANAGER_H_

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

            char * get()
            {
                if ( !str.get() )
                    str.reset( new PidBuffer( log_pid, log_tid ) );

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
            Logger & createLogger( std::string const & logName, size_t maxLevel = 0 );

            /// ������
            ILoggerStream & getStream( std::string const & streamName );

            /// ������־ģ��
            Logger & getLogger( std::string const & logName = "" );

            /// ����ģ��
            LogModule & getModule( std::string const & moduleName );

            /// �����
            void log_sec_beg() {++m_sec_ver_;}
            void log_sec_end() {++m_sec_ver_;}

        public:
            static size_t m_sec_ver_;
            static LoggerDefine * m_logdefine_;

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
