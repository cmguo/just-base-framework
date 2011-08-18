// LoggerManager.cpp

#include "framework/Framework.h"
#include "framework/logger/LoggerManager.h"
#include "framework/logger/Logger.h"
#include "framework/configure/Config.h"
#include "framework/logger/LoggerStream.h"
#include "framework/logger/LoggerStdStream.h"
#include "framework/logger/LoggerFileStream.h"

#include "framework/string/StringToken.h"

using namespace framework::string;
using namespace framework::configure;
using namespace framework::system::logic_error;

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/thread/mutex.hpp>
using namespace boost::system;

//#define INSERT_HEAD( listhead, node ) { listhead == NULL ? listhead = node : node->next = listhead, listhead = node; }

#ifdef BOOST_WINDOWS_API
#else
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#define gettid() pthread_self()
#endif

namespace framework
{
    namespace logger
    {
        //---------------------------------------------------------------------
        // struct PidBuffer

        /// 初始化缓冲区
        void PidBuffer::init_data( bool havepid, bool havetid )
        {
            // 格式化buffer
            if ( havepid && havetid )
            {
#ifdef BOOST_WINDOWS_API
                unsigned long pid = ::GetCurrentProcessId();
                unsigned long tid = ::GetCurrentThreadId();
                sprintf_s( pid_buffer_, 32, "] [%ld] [%ld] ", pid, tid );
#else
                pid_t pid = getpid();
                unsigned long tid = gettid();
                snprintf( pid_buffer_, 32, "] [%d] [%lu] ", pid, tid );
#endif
            }
            else if ( havepid )
            {
#ifdef BOOST_WINDOWS_API
                unsigned long pid = ::GetCurrentProcessId();
                sprintf_s( pid_buffer_, 32, "] [%ld] ", pid );
#else
                pid_t pid = getpid();
                snprintf( pid_buffer_, 32, "] [%d] ", pid );
#endif
            }
            else if ( havetid )
            {
#ifdef BOOST_WINDOWS_API
                unsigned long tid = ::GetCurrentThreadId();
                sprintf_s( pid_buffer_, 32, "] [%ld] ", tid );
#else
                unsigned long tid = gettid();
                snprintf( pid_buffer_, 32, "] [%lu] ", tid );
#endif
            }
            else
            {
                pid_buffer_[0] = ']';
                pid_buffer_[1] = ' ';
                pid_buffer_[2] = '\0';
            }
        }

        //---------------------------------------------------------------------
        // class LoggerManager
        /// 构造、析构
        LoggerManager::LoggerManager()
            : m_log_streams_( NULL )
            , m_loggers_( NULL )
            , m_logmodules_( NULL )
            , m_default_logger_( NULL )
            , m_sec_ver_( 0 )
        {
            m_logdefine_ = new LoggerDefine;
        }

        LoggerManager::~LoggerManager()
        {
            if ( m_logdefine_ )
            {
                delete m_logdefine_;
                m_logdefine_ = NULL;
            }

            while ( LogModule * lm = m_logmodules_ )
            {
                m_logmodules_ = m_logmodules_->next;
                delete lm;
                lm = NULL;
            }

            while ( Logger * lg = m_loggers_ )
            {
                m_loggers_ = m_loggers_->next;
                delete lg;
                lg = 0;
            }

            while ( ILoggerStream * ils = m_log_streams_ )
            {
                m_log_streams_ = m_log_streams_->next;
                delete ils;
                ils = NULL;
            }
        }

        /// 从配置文件读取每个输出流，创建输出流队列
        boost::system::error_code LoggerManager::load_config( 
            framework::configure::Config & conf )
        {
            ConfigModule & logger_mgr_config = conf.register_module( "LoggerManager" );

            /// 进程ID、线程ID打印标识
            logger_mgr_config << CONFIG_PARAM_NAME_RDWR( "log_pid", m_logdefine_->log_pid );
            logger_mgr_config << CONFIG_PARAM_NAME_RDWR( "log_tid", m_logdefine_->log_tid );

            // 初始化流链表，先于日志模块初始化
            initStreamList( conf );

            std::string defaultLog;
            /// 读取默认日志配置
            logger_mgr_config << CONFIG_PARAM_NAME_RDWR( "log_default", defaultLog );

            std::string logStreams;
            Logger * lg = NULL;

            if ( defaultLog.empty() )
            {
                lg = new Logger( *this, "Default" );
            }
            else
            {
                std::string tmp = "Logger_" + defaultLog;
                conf.register_module( tmp ) << CONFIG_PARAM_NAME_RDONLY( "logstreams", logStreams );

                lg = new Logger( *this, defaultLog );

                std::string stream;
                boost::system::error_code ec;
                StringToken strToken( logStreams, ";", true );

                strToken.next_token( stream, ec );
                while ( !ec )
                {
                    ILoggerStream * streamPtr = m_log_streams_;
                    while ( streamPtr )
                    {
                        if ( streamPtr->stream_name_ == stream )
                        {/// 将流插入到日志模块中
                            lg->insertStream( streamPtr );
                            break;
                        }
                        streamPtr = streamPtr->next;
                    }
                    strToken.next_token( stream, ec );
                }
            }

            m_default_logger_ = lg;

            /// 插入日志链头部
            insertLoggerList( lg );

            // 初始化日志链表
            initLoggerList( conf );

            // 初始化模块
            initModuleTree( conf );

            return succeed;
        }

        /// 注册模块
        LogModule & LoggerManager::register_module(
            char const * name, 
            size_t level,
            char const * log_name)
        {
            assert( m_default_logger_ );

            LogModule * lm = NULL;
            if ( !log_name )
            {
                lm = new LogModule( m_default_logger_, name, level );
            }
            else
            {
                Logger * log = &( getLogger( log_name ) );
                lm = (log) ? new LogModule( log, name, level ) : new LogModule( m_default_logger_, name, level );
            }

            /// 根据已经注册的模块信息，对代码中的信息进行覆盖
            std::map<std::string, std::map<std::string, std::string> >::iterator it = m_module_info_.find( name );
            if ( it != m_module_info_.end() )
            {
                std::map<std::string, std::string>::iterator its = it->second.find( "logger" );

                if ( its == it->second.end() )
                {
                    lm->logger_ = m_default_logger_;
                }
                else
                {
                    Logger * log = &( getLogger( its->second ) );
                    lm->logger_ = (log) ? log : m_default_logger_;/// 配置文件中的日志不存在，使用默认的
                }

                its = it->second.find( "level" );
                lm->level = atoi( ( its == it->second.end() ? "6" : its->second.c_str() ) );
            }

            return *( insertModuleList( lm ) );
        }

        ///.添加一个流
        ILoggerStream & LoggerManager::add_stream( ILoggerStream * ls )
        {
            return *( insertStreamList( ls ) );
        }

        ///.添加一个流到指定日志
        bool LoggerManager::add_stream_to( ILoggerStream * ls, std::string const & logname  )
        {
            if ( !ls ) return false;

            if ( !insertStreamList(ls) ) return false;
            Logger &log = getLogger( logname );
            if ( !(&log) ) return false;
            log.insertStream( ls );
            return true;
        }

        /// 删除一个流
        bool LoggerManager::del_stream( ILoggerStream * ls )
        {
            /// 加写锁
            if ( !m_log_streams_ ) return true;
            ILoggerStream **pre = &m_log_streams_;

            while ( *pre )
            {
                if ( (*pre) == ls ) 
                {/// 并不实际删除，析构的时候统一删除
                    (*pre)->is_del_ = true;
                    //ILoggerStream *p = *pre;
                    //*pre = p->next;
                    //delete p;
                    break;
                }
                pre = &(*pre)->next;
            }

            /// 删除日志中引用的流
            if ( m_loggers_ ) 
            {
                Logger *lg = m_loggers_;
                while ( lg )
                {
                    lg->delStream( ls );
                    lg = lg->next;
                }
            }

            return true;
        }

        /// 创建一个日志模块
        Logger & LoggerManager::createLogger( std::string const & logName )
        {
            Logger * lg = new Logger( *this, logName );
            return *( insertLoggerList( lg ) );
        }

        /// 查找流
        ILoggerStream & LoggerManager::getStream( std::string const & streamName )
        {
            /// 加读锁
            ILoggerStream * cur = m_log_streams_;
            while ( cur )
            {
                if ( cur->stream_name_ == streamName && !cur->is_del_ ) return *( cur );
                cur = cur->next;
            }
            return *( cur );
        }

        /// 查找日志模块
        Logger & LoggerManager::getLogger( std::string const & logName )
        {
            if ( logName.empty() ) return *(m_default_logger_);

            Logger * cur = m_loggers_;
            while ( cur )
            {
                if ( cur->m_log_name_ == logName ) return *( cur );
                cur = cur->next;
            }
            return *( cur );
        }

        /// 查找模块
        LogModule & LoggerManager::getModule( std::string const & moduleName )
        {
            LogModule * cur = m_logmodules_;
            while ( cur )
            {
                if ( moduleName == cur->name ) return *( cur );
                cur = cur->next;
            }
            return *( cur );
        }

        /// 插入输出流链表
        ILoggerStream * LoggerManager::insertStreamList( ILoggerStream * ls )
        {
            if ( !ls ) return NULL;

            boost::mutex::scoped_lock lock( m_stream_mutex_ );
            m_log_streams_ == NULL ? m_log_streams_ = ls : ls->next = m_log_streams_, m_log_streams_ = ls;

            return m_log_streams_;
        }

        /// 插入日志链表
        Logger * LoggerManager::insertLoggerList( Logger * lg )
        {
            if ( !lg ) return NULL;

            boost::mutex::scoped_lock lock( m_logger_mutex_ );
            m_loggers_ == NULL ? m_loggers_ = lg : lg->next = m_loggers_, m_loggers_ = lg;

            return m_loggers_;
        }

        /// 插入模块链表
        LogModule * LoggerManager::insertModuleList( LogModule * lm )
        {
            if ( !lm ) return NULL;

            boost::mutex::scoped_lock lock( m_module_mutex_ );
            m_logmodules_ == NULL ? m_logmodules_ = lm : lm->next = m_logmodules_, m_logmodules_ = lm;

            return m_logmodules_;
        }

        /// 初始化输出流链表
        void LoggerManager::initStreamList( framework::configure::Config & conf )
        {
            std::map<std::string, std::map<std::string, std::string> > mkvs;
            conf.profile().get_all( mkvs );
            std::map<std::string, std::map<std::string, std::string> >::iterator it = mkvs.begin();
            for ( ; it != mkvs.end(); ++it )
            {
                std::string::size_type pos = it->first.find( "_" );
                if ( pos != std::string::npos && it->first.substr( 0, pos ) == "LogStream" )
                {
                    std::string logStreamType;
                    size_t logStreamLevel = kLevelNone;
                    ILoggerStream * ls = NULL;

                    conf.register_module( it->first ) << CONFIG_PARAM_NAME_RDONLY( "type", logStreamType )
                        << CONFIG_PARAM_NAME_RDONLY( "level", logStreamLevel );

                    if ( "STDOUT" == logStreamType )
                    {
                        bool haveColor = false;
                        conf.register_module( it->first ) << CONFIG_PARAM_NAME_RDONLY( "color", haveColor );
                        ls = new LoggerStdStream( haveColor );
                    }
                    else if ( "FILE" == logStreamType )
                    {
                        std::string fileName;
                        size_t fileSize = 0;
                        bool isApp = false;
                        bool isDay = false;
                        bool isRoll = false;
                        conf.register_module( it->first ) << CONFIG_PARAM_NAME_RDONLY( "file", fileName )
                            << CONFIG_PARAM_NAME_RDONLY( "size", fileSize )
                            << CONFIG_PARAM_NAME_RDONLY( "append", isApp )
                            << CONFIG_PARAM_NAME_RDONLY( "daily", isDay )
                            << CONFIG_PARAM_NAME_RDONLY( "roll", isRoll );
                        ls = new LoggerFileStream( fileName, fileSize, isApp, isDay, isRoll );
                    }
                    else
                    {
                        ls = new LoggerStdStream( false );
                        // any other streams ...
                    }

                    ls->stream_name_ = it->first.substr( pos + 1, it->first.size() - pos );
                    ls->stream_type_ = logStreamType;
                    ls->log_lvl = logStreamLevel;
                    ls->next = NULL;

                    /// 加入流链表头部
                    insertStreamList( ls );
                }
            }
        }

        /// 初始化日志链表
        void LoggerManager::initLoggerList( framework::configure::Config & conf )
        {
            std::map<std::string, std::map<std::string, std::string> > mkvs;
            conf.profile().get_all( mkvs );
            std::map<std::string, std::map<std::string, std::string> >::iterator it = mkvs.begin();
            for ( ; it != mkvs.end(); ++it )
            {
                std::string::size_type pos = it->first.find( "_" );
                if ( pos != std::string::npos 
                    && it->first.substr( 0, pos ) == "Logger" 
                    && it->first.substr( pos + 1, it->first.size() -  pos ) != m_default_logger_->m_log_name_ )
                {
                    std::string logStreams;
                    conf.register_module( it->first ) << CONFIG_PARAM_NAME_RDONLY( "logstreams", logStreams );

                    Logger * lg = new Logger( *this, it->first.substr( pos + 1, it->first.size() - pos ) );

                    std::string stream;
                    boost::system::error_code ec;
                    StringToken strToken( logStreams, ";" );
                    strToken.next_token( stream, ec );
                    while ( !ec )
                    {
                        ILoggerStream * streamPtr = m_log_streams_;
                        while ( streamPtr )
                        {
                            if ( streamPtr->stream_name_ == stream )
                            {/// 将流插入到日志模块中
                                /// 需要保存当前所有流中最大等级*******
                                lg->insertStream( streamPtr );
                                break;
                            }
                            streamPtr = streamPtr->next;
                        }
                        strToken.next_token( stream, ec );
                    }

                    /// 插入日志链头部
                    insertLoggerList( lg );
                }
            }
        }

        /// 初始化模块结构
        void LoggerManager::initModuleTree( framework::configure::Config & conf )
        {
            /// 首先将各个模块序列化到map中
            std::map<std::string, std::map<std::string, std::string> > mkvs, mods;
            conf.profile().get_all( mkvs );
            std::map<std::string, std::map<std::string, std::string> >::iterator it, its, it1;
            for ( it = mkvs.begin(); it != mkvs.end(); ++it )
            {
                std::string::size_type pos = it->first.find( "_" );
                if ( pos != std::string::npos 
                    && it->first.substr( 0, pos ) == "LogModule" )
                {
                    std::string modlvl;
                    std::string modlog;
                    conf.register_module( it->first ) << CONFIG_PARAM_NAME_RDONLY( "logger", modlog )
                        << CONFIG_PARAM_NAME_RDONLY( "level", modlvl );
                    
                    std::string modname = it->first.substr( pos + 1, it->first.size() - pos );
                    if ( modname.empty() )
                    {
                        std::map< std::string, std::string > keyval;
                        keyval["logger"] = modlog;
                        keyval["level"] = modlvl;
                        mods["."] = keyval;
                    }
                    else
                    {
                        std::map< std::string, std::string > keyval;
                        keyval["logger"] = modlog;
                        keyval["level"] = modlvl;
                        mods[modname] = keyval;
                    }
                }
            }

            /// 初始化根模块的绑定日志和等级信息
            std::string rootlog, rootlvl;
            it1 = mods.find( "." );
            if ( it1 != mods.end() )
            {
                std::map< std::string , std::string >::iterator iters = it1->second.find( "logger" );
                rootlog = iters != it1->second.end() ? iters->second : rootlog;
                iters = it1->second.find( "level" );
                rootlvl = iters != it1->second.end() ? iters->second : rootlvl;
            }

            /// 分析各个子模块，序列化到map
            for ( its = mods.begin(); its != mods.end(); ++its )
            {
                if ( its->first == "." )
                {/// 不再处理根模块信息
                    continue;
                }
                std::string log = rootlog, lvl = rootlvl;
                std::string submod;
                std::string::size_type lastpos = 0, pos = 0;
                pos = its->first.find( ".", lastpos );
                while ( pos != std::string::npos )
                {
                    lastpos = pos;
                    submod = its->first.substr( 0, pos );
                    it1 = mods.find( submod );
                    if ( it1 != mods.end() )
                    {
                        std::map< std::string , std::string >::iterator iters = it1->second.find( "logger" );
                        log = ( iters == it1->second.end() || iters->second.empty() ) ? log : iters->second;
                        iters = it1->second.find( "level" );
                        lvl = ( iters == it1->second.end() || iters->second.empty() ) ? lvl : iters->second;
                    }
                    pos = its->first.find( ".", lastpos + 1 );
                }

                it1 = mods.find( its->first );
                if ( it1 != mods.end() )
                {
                    std::map< std::string , std::string >::iterator iters = it1->second.find( "logger" );
                    log = ( iters == it1->second.end() || iters->second.empty() ) ? log : iters->second;
                    iters = it1->second.find( "level" );
                    lvl = ( iters == it1->second.end() || iters->second.empty() ) ? lvl : iters->second;
                }

                std::string leaf = its->first.substr( lastpos ? lastpos + 1 : lastpos, its->first.size() - lastpos );

                std::map< std::string, std::string > keyval;
                keyval["logger"] = log;
                keyval["level"] = lvl;
                m_module_info_[leaf] = keyval;
            }
        }

        LoggerManager & global_logger_mgr()
        {
            static LoggerManager slog_mgr_;
            return slog_mgr_;
        }

        LoggerManager & glog_mgr = global_logger_mgr();

    } // namespace logger
} // namespace framework
