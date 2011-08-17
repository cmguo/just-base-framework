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

            char * get()
            {
                if ( !str.get() )
                    str.reset( new PidBuffer( log_pid, log_tid ) );

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
            Logger & createLogger( std::string const & logName, size_t maxLevel = 0 );

            /// 查找流
            ILoggerStream & getStream( std::string const & streamName );

            /// 查找日志模块
            Logger & getLogger( std::string const & logName = "" );

            /// 查找模块
            LogModule & getModule( std::string const & moduleName );

            /// 段输出
            void log_sec_beg() {++m_sec_ver_;}
            void log_sec_end() {++m_sec_ver_;}

        public:
            static size_t m_sec_ver_;
            static LoggerDefine * m_logdefine_;

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
