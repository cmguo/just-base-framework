// LoggerFormatString.h.h

#ifndef _FRAMEWORK_LOGGER_LOGGERFORMATSTRING_H_
#define _FRAMEWORK_LOGGER_LOGGERFORMATSTRING_H_

#include "framework/logger/LoggerFormatStreambuff.h"

namespace framework
{
    namespace logger
    {
        /// 支持的参数个数
        const int PARA_NUM = 50;
        /// 格式化外部缓冲区
        class LoggerFormatString
        {
        public:
            LoggerFormatString( char * buf, std::size_t bufsize, std::size_t datasize = 0 )
            {
                mp_stream_buf_ = new FormatStreambuf( buf, bufsize, datasize );
                mp_oss_ = new std::ostream(mp_stream_buf_);
                m_curPos = 0;
                m_curIndex = 0;
                m_totalParam = 0;
            }

            ~LoggerFormatString()
            {
                if ( mp_stream_buf_ )
                {
                    delete mp_stream_buf_;
                    mp_stream_buf_ = NULL;
                }
                if ( mp_oss_ )
                {
                    delete mp_oss_;
                    mp_oss_ = NULL;
                }
            }

            size_t size()
            {
                return mp_stream_buf_->size();
            }

            template< class Ty_ >
            LoggerFormatString & operator % ( Ty_ value_ )
            {
                mp_oss_->write( &(mp_fmt_[m_curPos]), m_pos[m_curIndex] - m_curPos );
                //for ( int i = m_curPos; i < m_pos[m_curIndex]; ++i )
                //    *mp_oss_ << mp_fmt_[i];

                *mp_oss_ << value_;
                m_curPos = m_pos[m_curIndex] + 2;
                m_curIndex++;

                if ( m_curIndex == m_totalParam )
                {
                    mp_oss_->write( &( mp_fmt_[m_curPos] ), strlen( mp_fmt_ ) - m_curPos );
                    //for ( int i = m_curPos; i < strlen( mp_fmt_ ); ++i )
                    //    *mp_oss_ << mp_fmt_[i];
                }

                return *this;
            }

            LoggerFormatString & parse ( char const * fmt )
            {
                if ( !strlen( fmt ) ) return *this;
                mp_fmt_ = fmt;

                for ( size_t i = 0; i < strlen( fmt ); ++i )
                {// 解析每个参数位置
                    if ( mp_fmt_[i] == '%' )
                    {
                        m_pos[m_totalParam++] = i;
                        i++;
                    }
                }
                return *this;
            }

        private:
            FormatStreambuf * mp_stream_buf_;
            std::ostream * mp_oss_;
            char const * mp_fmt_;   /// 格式化串
            size_t m_pos[PARA_NUM]; /// 所有参数的位置
            size_t m_curPos;        /// 当前串中位置
            size_t m_curIndex;      /// 当前下标
            size_t m_totalParam;    /// 参数总数
        };
    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_LOGGERFORMATSTRING_H_
