// Logger.cpp

#include "framework/Framework.h"
#include "framework/logger/Logger.h"
#include "framework/logger/LoggerStream.h"

#include <stdio.h>

namespace framework
{
    namespace logger
    {

#define DATE_START  1
#define TIME_START  (DATE_START + 11)

        /// 错误等级字符串
        char const * const gp_level_str[] = {"[ERROR] [", "[ALARM] [", "[EVENT] [", "[INFOR] [", "[DEBUG] [", "[DEBUG] [", "[DEBUG] ["};
        const size_t lvl_str_len = 9;
        const size_t time_str_len = 22;
        char const * const gp_newline = "\n";
        char const * line_fmt = "%08Xh:  %X%X %X%X %X%X %X%X  %X%X %X%X %X%X %X%X  %X%X %X%X %X%X %X%X  %X%X %X%X %X%X %X%X\n";

        Logger::Logger( LoggerManager & logmgr, std::string const & name )
            : next( NULL )
            , m_log_name_( name )
            , m_max_level_( kLevelNone )
            , mp_log_streams_( NULL )
            , m_log_mgr_( logmgr )

        {
            strncpy(time_str_, "<0000-00-00 00:00:00>", sizeof(time_str_));
            ILoggerStream::time_str_now(time_str_ + DATE_START, 11, "%Y-%m-%d");
            time_t tt = time(NULL);
            struct tm lt;
            localtime_r(&tt, &lt);
            lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
            mid_night_ = mktime(&lt);
        }

        Logger::~Logger()
        {
            while ( LoggerStreams * lss = mp_log_streams_ )
            {
                mp_log_streams_ = mp_log_streams_->next;
                delete lss;
                lss = NULL;
            }
        }

        /// 按照从大到小的顺序，插入一个流
        void Logger::insertStream( ILoggerStream * ls )
        {
            if ( !ls ) return;

            boost::mutex::scoped_lock lock( streams_mutex_ );

            if ( m_max_level_ == kLevelNone ) m_max_level_ = ls->log_lvl;
            else
                m_max_level_ = ls->log_lvl > m_max_level_ ? ls->log_lvl : m_max_level_;

            if ( !mp_log_streams_ )
            {
                mp_log_streams_ = new LoggerStreams( ls );
            }
            else
            {
                LoggerStreams * pre = mp_log_streams_, * next = mp_log_streams_;
                while ( next )
                {
                    if ( next->is_del_ && next->stream_ == ls )
                    {/// 首先判断当前节点的流是否是以前删除的
                        next->is_del_ = false;
                        break;
                    }

                    if ( next->stream_->log_lvl < ls->log_lvl )
                    {
                        if ( pre == next )
                        {/// 插入链头
                            mp_log_streams_ = new LoggerStreams( ls );
                            mp_log_streams_->next = pre;
                        }
                        else
                        {/// 插入链中
                            pre->next = new LoggerStreams( ls );
                            pre->next->next = next;
                        }
                        break;
                    }
                    pre = next;
                    next = next->next;
                }
                if ( !next )
                {/// 插入到链尾
                    pre->next = new LoggerStreams( ls );
                }
            }
        }

        /// 删除一个流
        void Logger::delStream( ILoggerStream * ls )
        {
            if ( !ls || !mp_log_streams_ ) return;
            LoggerStreams **pre = &mp_log_streams_;

            while ( *pre )
            {
                if ( (*pre)->stream_ == ls ) {
                    (*pre)->is_del_ = true; /// 标记被删除
                    break;
                }
                pre = &(*pre)->next;
            }
        }

        /// 输出日志
        void Logger::printLog(
            LogModule const & module, 
            size_t level, 
            LoggerRecord const & record)
        {
            LoggerStreams * cur = mp_log_streams_;
            while ( cur && cur->stream_->log_lvl >= level )
            {/// 内部格式化串
                if ( cur->is_del_ || cur->stream_->is_del_ ) 
                {/// 当前流已经被删除
                    cur = cur->next;
                    continue;
                }

                if ( cur->stream_->stream_ver != m_log_mgr_.getVer() )
                {
                    if ( time_ver_ != m_log_mgr_.getVer() )
                    {
                        int t_diff = (int)(time(NULL) - mid_night_);
                        if (t_diff >= 24 * 60 * 60) 
                        {
                            ILoggerStream::time_str_now(time_str_ + DATE_START, 11, "%Y-%m-%d");
                            mid_night_ += 24 * 60 * 60;
                            t_diff -= 24 * 60 * 60;
                        }

                        int hour = t_diff / 3600;
                        int minute = t_diff % 3600;
                        int second = minute % 60;
                        minute /= 60;
                        time_str_[DATE_START + 10] = ' ';
                        time_str_[TIME_START] = hour / 10 + '0';
                        time_str_[TIME_START + 1] = hour % 10 + '0';
                        time_str_[TIME_START + 3] = minute / 10 + '0';
                        time_str_[TIME_START + 4] = minute % 10 + '0';
                        time_str_[TIME_START + 6] = second / 10 + '0';
                        time_str_[TIME_START + 7] = second % 10 + '0';
                        time_str_[TIME_START + 9] = '\n';

                        time_ver_ = m_log_mgr_.getVer();
                    }
                    ILoggerStream::muti_buffer_t buffers[1];
                    buffers[0].buffer   = time_str_;
                    buffers[0].len      = time_str_len;

                    cur->stream_->write( buffers, 1 );

                    cur->stream_->stream_ver = m_log_mgr_.getVer();
                }

                ILoggerStream::muti_buffer_t buffers[4];
                char msg[1024] = { 0 };
                size_t len = record.format_message( msg, sizeof( msg ) - 1 );
                buffers[0].buffer   = gp_level_str[level];
                buffers[0].len      = lvl_str_len;
                buffers[1].buffer   = module.name;
                buffers[1].len      = strlen( module.name );
                buffers[2].buffer   = m_log_mgr_.getLoggerDefine().get();
                buffers[2].len      = m_log_mgr_.getLoggerDefine().str_len;
                buffers[3].buffer   = msg;
                buffers[3].len      = len;
                cur->stream_->write( buffers, 4 );

                cur = cur->next;
            }
        }

        /// 字符串打印
        void Logger::printString(
            LogModule const & module,
            size_t level, 
            char const * txt, 
            size_t len)
        {
            LoggerStreams * cur = mp_log_streams_;
            while ( cur && cur->stream_->log_lvl >= level )
            {
                if ( cur->is_del_ || cur->stream_->is_del_ ) 
                {/// 当前流已经被删除
                    cur = cur->next;
                    continue;
                }

                ILoggerStream::muti_buffer_t buffers[5];
                buffers[0].buffer   = gp_level_str[level];
                buffers[0].len      = lvl_str_len;
                buffers[1].buffer   = module.name;
                buffers[1].len      = strlen( module.name );
                buffers[2].buffer   = m_log_mgr_.getLoggerDefine().get();
                buffers[2].len      = m_log_mgr_.getLoggerDefine().str_len;
                buffers[3].buffer   = txt;
                buffers[3].len      = len ? len : strlen( txt );
                buffers[4].buffer   = gp_newline;
                buffers[4].len      = 1;
                cur->stream_->write( buffers, 5 );

                cur = cur->next;
            }
        }

        static char const chex[] = "0123456789ABCDEF";

        /// 打印十六进制
        void Logger::printHex(
            LogModule const & module,
            size_t level, 
            unsigned char const * data, 
            size_t len)
        {
            LoggerStreams * cur = mp_log_streams_;
            while ( cur && cur->stream_->log_lvl >= level )
            {
                if ( cur->is_del_ || cur->stream_->is_del_ ) 
                {/// 当前流已经被删除
                    cur = cur->next;
                    continue;
                }

                /// 打印日志等级、模块名称等信息
                ILoggerStream::muti_buffer_t buffers[4];
                buffers[0].buffer   = gp_level_str[level];
                buffers[0].len      = lvl_str_len;
                buffers[1].buffer   = module.name;
                buffers[1].len      = strlen( module.name );
                buffers[2].buffer   = m_log_mgr_.getLoggerDefine().get();
                buffers[2].len      = m_log_mgr_.getLoggerDefine().str_len;
                buffers[3].buffer   = gp_newline;
                buffers[3].len      = 1;
                cur->stream_->write( buffers, 4 );

                cur = cur->next;
            }

            const size_t line_num_len = 4;
            const size_t line_num_start = 1;
            const size_t hex_start = line_num_len + 2;
            //const size_t text_start = hex_start + 52 + 1;

            size_t line, byte, line_pos, data_pos;

            // 填入不变的部分
            char msg_str_[1024] = { 0 };
            line_pos = 0;
            msg_str_[line_pos++] = '[';
            line_pos += line_num_len;
            msg_str_[line_pos++] = ']';
            for (byte = 0; byte < 16; byte++)
            {
                if ((byte & 0x03) == 0)
                {
                    msg_str_[line_pos++] = ' ';
                }
                line_pos += 2;
                msg_str_[line_pos++] = ' ';
            }
            msg_str_[line_pos++] = '|';
            line_pos += 16;
            msg_str_[line_pos++] = '|';
            msg_str_[line_pos] = 0;

            // 开始工作
            data_pos = 0;
            for (line = 0; line < len / 16; line++)
            {
                size_t n = line;
                for (size_t i = line_num_len - 1; i != (size_t)-1; i--)
                {
                    msg_str_[line_num_start + i] = chex[n & 0x0F];
                    n >>= 4;
                }
                line_pos = hex_start;
                for (byte = 0; byte < 16; byte++, data_pos++, line_pos++)
                {
                    if ((byte & 0x03) == 0)
                    {
                        line_pos++;
                    }
                    msg_str_[line_pos++] = chex[data[data_pos] >> 4];
                    msg_str_[line_pos++] = chex[data[data_pos] & 0x0F];
                }
                line_pos++; // skip border '|'
                for (data_pos -= 16, byte = 0; byte < 16; byte++, data_pos++, line_pos++)
                {
                    if (isgraph(data[data_pos]))
                        msg_str_[line_pos] = data[data_pos];
                    else
                        msg_str_[line_pos] = '.';
                }

                cur = mp_log_streams_;
                while ( cur && cur->stream_->log_lvl >= level )
                {
                    if ( cur->is_del_ || cur->stream_->is_del_ ) 
                    {/// 当前流已经被删除
                        cur = cur->next;
                        continue;
                    }

                    ILoggerStream::muti_buffer_t buffers[1];
                    char szbuf[1024] = { 0 };
                    snprintf(szbuf, sizeof( szbuf ), "# %s\n", msg_str_);
                    buffers[0].buffer   = szbuf;
                    buffers[0].len      = strlen( szbuf );
                    cur->stream_->write( buffers, 1 );

                    cur = cur->next;
                }
            }
            size_t n = line;
            for (size_t i = line_num_len - 1; i != (size_t)-1; i--) 
            {
                msg_str_[line_num_start + i] = chex[n & 0x0F];
                n >>= 4;
            }
            for (line_pos = hex_start, byte = 0; byte < len % 16; byte++, data_pos++, line_pos++) 
            {
                if ((byte & 0x03) == 0)
                {
                    line_pos++;
                }
                msg_str_[line_pos++] = chex[data[data_pos] >> 4];
                msg_str_[line_pos++] = chex[data[data_pos] & 0x0F];
            }
            for (data_pos -= len % 16; byte < 16; byte++, line_pos++)
            {
                if ((byte & 0x03) == 0)
                {
                    line_pos++;
                }
                msg_str_[line_pos++] = ' ';
                msg_str_[line_pos++] = ' ';
            }
            line_pos++; // skip border '|'
            for (byte = 0; byte < len % 16; byte++, data_pos++, line_pos++)
            {
                if (isgraph(data[data_pos]))
                    msg_str_[line_pos] = data[data_pos];
                else
                    msg_str_[line_pos] = '.';
            }
            for (; byte < 16; byte++, line_pos++)
            {
                msg_str_[line_pos] = ' ';
            }

            cur = mp_log_streams_;
            while ( cur && cur->stream_->log_lvl >= level )
            {
                if ( cur->is_del_ || cur->stream_->is_del_ ) 
                {/// 当前流已经被删除
                    cur = cur->next;
                    continue;
                }

                ILoggerStream::muti_buffer_t buffers[1];
                char szbuf[1024] = { 0 };
                snprintf(szbuf, sizeof( szbuf ), "# %s\n", msg_str_);
                buffers[0].buffer   = szbuf;
                buffers[0].len      = strlen( szbuf );
                cur->stream_->write( buffers, 1 );

                cur = cur->next;
            }

            //LoggerStreams * cur = mp_log_streams_;
            //while ( cur && cur->stream_->log_lvl >= level )
            //{
            //    if ( define->log_pid || define->log_tid )
            //    {
            //        ILoggerStream::muti_buffer_t buffers[4];
            //        buffers[0].buffer   = gp_level_str[level];
            //        buffers[0].len      = strlen( gp_level_str[level] );
            //        buffers[1].buffer   = module.name;
            //        buffers[1].len      = strlen( module.name );
            //        buffers[2].buffer   = define->get();
            //        buffers[2].len      = strlen( define->get() );
            //        buffers[3].buffer   = gp_newline;
            //        buffers[3].len      = strlen( gp_newline );
            //        cur->stream_->write( buffers, 4 );
            //    }
            //    else
            //    {
            //        ILoggerStream::muti_buffer_t buffers[3];
            //        buffers[0].buffer   = gp_level_str[level];
            //        buffers[0].len      = strlen( gp_level_str[level] );
            //        buffers[1].buffer   = module.name;
            //        buffers[1].len      = strlen( module.name );
            //        buffers[2].buffer   = gp_newline;
            //        buffers[2].len      = strlen( gp_newline );
            //        cur->stream_->write( buffers, 3 );
            //    }
            //    cur = cur->next;
            //}

            //char line_buf[64][64] = { 0 };
            //size_t count = 0, cur_pos = 0;
            //size_t all_lines = len / 16;
            //bool isremain = len % 16 ? true : false;
            //for ( size_t line = 0; line < all_lines && cur_pos < len; ++line, cur_pos += 16 )
            //{
            //    if ( count > 63  )
            //    {
            //        ILoggerStream::muti_buffer_t buffers[64];
            //        for ( size_t i = 0; i < count; ++i )
            //        {
            //            buffers[i].buffer = line_buf[i];
            //            buffers[i].len = strlen( line_buf[i] );
            //        }
            //        LoggerStreams * cur = mp_log_streams_;
            //        while ( cur && cur->stream_->log_lvl >= level )
            //        {
            //            cur->stream_->write( buffers, count );
            //            cur = cur->next;
            //        }

            //        count = 0;
            //    }
            //    else
            //    {
            //        snprintf( line_buf[line % 64]
            //        , sizeof( line_buf ) / 64
            //            , line_fmt
            //            , line * 16
            //            , data[ line * 16 + 0 ] >> 4 & 0x0F
            //            , data[ line * 16 + 0 ] & 0x0F
            //            , data[ line * 16 + 1 ] >> 4 & 0x0F 
            //            , data[ line * 16 + 1 ] & 0x0F
            //            , data[ line * 16 + 2 ] >> 4 & 0x0F
            //            , data[ line * 16 + 2 ] & 0x0F  
            //            , data[ line * 16 + 3 ] >> 4 & 0x0F
            //            , data[ line * 16 + 3 ] & 0x0F
            //            , data[ line * 16 + 4 ] >> 4 & 0x0F  
            //            , data[ line * 16 + 4 ] & 0x0F
            //            , data[ line * 16 + 5 ] >> 4 & 0x0F  
            //            , data[ line * 16 + 5 ] & 0x0F
            //            , data[ line * 16 + 6 ] >> 4 & 0x0F  
            //            , data[ line * 16 + 6 ] & 0x0F
            //            , data[ line * 16 + 7 ] >> 4 & 0x0F  
            //            , data[ line * 16 + 7 ] & 0x0F
            //            , data[ line * 16 + 8 ] >> 4 & 0x0F  
            //            , data[ line * 16 + 8 ] & 0x0F
            //            , data[ line * 16 + 9 ] >> 4 & 0x0F  
            //            , data[ line * 16 + 9 ] & 0x0F
            //            , data[ line * 16 + 10] >> 4 & 0x0F  
            //            , data[ line * 16 + 10] & 0x0F
            //            , data[ line * 16 + 11] >> 4 & 0x0F  
            //            , data[ line * 16 + 11] & 0x0F
            //            , data[ line * 16 + 12] >> 4 & 0x0F
            //            , data[ line * 16 + 12] & 0x0F
            //            , data[ line * 16 + 13] >> 4 & 0x0F  
            //            , data[ line * 16 + 13] & 0x0F
            //            , data[ line * 16 + 14] >> 4 & 0x0F  
            //            , data[ line * 16 + 14] & 0x0F
            //            , data[ line * 16 + 15] >> 4 & 0x0F  	
            //            , data[ line * 16 + 15] & 0x0F
            //            );
            //        count++;
            //    }
            //}

            //if ( count < 64 )
            //{
            //    size_t i = 0;
            //    ILoggerStream::muti_buffer_t buffers[64];
            //    for (; i < count; ++i )
            //    {
            //        buffers[i].buffer = line_buf[i];
            //        buffers[i].len = strlen( line_buf[i] );
            //    }
            //    LoggerStreams * cur = mp_log_streams_;
            //    while ( cur && cur->stream_->log_lvl >= level )
            //    {
            //        cur->stream_->write( buffers, i );
            //        cur = cur->next;
            //    }
            //}

            //if ( isremain )
            //{ ///
            //    size_t other_len = len - cur_pos, sum = 0;
            //    char other_buf[24][16] = { 0 };
            //    snprintf( other_buf[sum++], 16, "%08Xh:  ", count * 16 );
            //    for ( size_t i = 0; i < other_len; ++i )
            //    {
            //        snprintf( other_buf[sum++], 16, "%X%X ",data[cur_pos] >> 4 & 0x0F, data[cur_pos] & 0x0F );
            //        cur_pos++;
            //        if ( !( ( i + 1 ) % 4 ) ) snprintf( other_buf[sum++], 16, " " );
            //    }
            //    snprintf( other_buf[sum++], 16, "\n" );
            //    ILoggerStream::muti_buffer_t buffers[24];
            //    for (size_t i = 0; i < sum; ++i )
            //    {
            //        buffers[i].buffer = other_buf[i];
            //        buffers[i].len = strlen( other_buf[i] );
            //    }
            //    LoggerStreams * cur = mp_log_streams_;
            //    while ( cur && cur->stream_->log_lvl >= level )
            //    {
            //        cur->stream_->write( buffers, sum );
            //        cur = cur->next;
            //    }
            //}

        }

    } // namespace logger
} // namespace framework
