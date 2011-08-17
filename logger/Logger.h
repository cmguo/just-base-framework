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
            enum
            {
                kLevelError     = 0,    // ��Ϣ����Ķ���
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

            Logger * next;                      /// ��һ����־
            std::string m_log_name_;            /// ��־����
            size_t mid_night_;
            size_t time_ver_;
            char time_str_[40];                 /// ���ڴ�
            size_t m_max_level_;                /// ��ǰָ���������ȼ������ڶ�������
            LoggerStreams * mp_log_streams_;    /// ӵ�е���

            Logger( std::string const & name, size_t level = kLevelNone );
            ~Logger();

            /// ����һ����
            void insertStream( ILoggerStream * ls );

            /// ɾ��һ����
            void delStream( ILoggerStream * ls );

            /// �����־
            void print_log(
                size_t ver,
                LoggerDefine * define,
                LogModule const & module, 
                size_t level, 
                LoggerRecord const & record);

            /// �ַ�����ӡ
            void print_string(
                LoggerDefine * define,
                LogModule const & module,
                size_t level, 
                char const * txt, 
                size_t len = 0);

            /// ��ӡʮ������
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
                    || ( level > module.level ) /// ����ģ������ȼ�����
                    || ( level > module.logger_->m_max_level_ ) )/// ���������ȼ�����
                    return;

                // ���ݵȼ�����õ������С������д�ӡ
                module.logger_->print_log( LoggerManager::m_sec_ver_, LoggerManager::m_logdefine_, module, level, record);
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
                    || ( level > module.level ) /// ����ģ������ȼ�����
                    || ( level > module.logger_->m_max_level_ ) )/// ���������ȼ�����
                    return;
                module.logger_->print_string( LoggerManager::m_logdefine_, module, level, txt, len );
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
