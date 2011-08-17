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
            enum
            {
                kLevelError     = 0,    // 消息级别的定义
                kLevelAlarm,
                kLevelEvent,
                kLevelInfor,
                kLevelDebug,
                kLevelDebug1,                kLevelDebug2,                kLevelNone            };

            struct LoggerStreams
            {
                LoggerStreams( ILoggerStream * stream ) : stream_( stream ), next( NULL ) {}
                ILoggerStream * stream_;
                LoggerStreams * next;
            };

            Logger * next;                      /// 下一个日志
            std::string m_log_name_;            /// 日志名称
            size_t mid_night_;
            size_t time_ver_;
            char time_str_[40];                 /// 日期串
            size_t m_max_level_;                /// 当前指向流的最大等级，用于二级过滤
            LoggerStreams * mp_log_streams_;    /// 拥有的流

            Logger( std::string const & name, size_t level = kLevelNone );
            ~Logger();

            /// 插入一个流
            void insertStream( ILoggerStream * ls );

            /// 删除一个流
            void delStream( ILoggerStream * ls );

            /// 输出日志
            void print_log(
                size_t ver,
                LoggerDefine * define,
                LogModule const & module, 
                size_t level, 
                LoggerRecord const & record);

            /// 字符串打印
            void print_string(
                LoggerDefine * define,
                LogModule const & module,
                size_t level, 
                char const * txt, 
                size_t len = 0);

            /// 打印十六进制
            void print_hex(
                LoggerDefine * define,
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
                module.logger_->print_log( LoggerManager::m_sec_ver_, LoggerManager::m_logdefine_, module, level, record);
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
                module.logger_->print_string( LoggerManager::m_logdefine_, module, level, txt );
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
                module.logger_->print_string( LoggerManager::m_logdefine_, module, level, txt, len );
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
                module.logger_->print_hex( LoggerManager::m_logdefine_, module, level, data, len );
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
