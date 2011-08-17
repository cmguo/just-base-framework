#ifndef _FRAMEWORK_LOGGER_LOGSTREAM_H_ 
#define _FRAMEWORK_LOGGER_LOGSTREAM_H_

#ifdef BOOST_WINDOWS_API
#  ifdef UNDER_CE
#    define localtime_r(x, y) *y = *localtime(x)
#  else 
#    define localtime_r(x, y) localtime_s(y, x)
#  endif
#  define snprintf _snprintf
#  define mkdir(x, y) mkdir(x)
#endif

namespace framework
{
    namespace logger
    {
        /// 输出流接口定义
        struct ILoggerStream
        {
            struct muti_buffer_t 
            {
                const char * buffer;
                size_t len;
            };

            ILoggerStream()
                : next( NULL )
                , log_lvl( 0 )
                , stream_ver( 0 )
                , is_del_( false )
            {
            }

            virtual ~ILoggerStream() {}

            virtual void write( 
                muti_buffer_t const * logmsgs, 
                size_t len = 0 ) = 0;

            static char const *time_str_now(
                char * buf, 
                size_t size, 
                char const * fmt)
            {
                time_t tt = time(NULL);
                struct tm lt;
                localtime_r(&tt, &lt);
                strftime(buf, size, fmt, &lt);
                return buf;
            }

            ILoggerStream * next;       /// 下一个流
            std::string stream_name_;   /// 名称
            std::string stream_type_;   /// 流类型
            size_t log_lvl;             /// 日志等级
            size_t stream_ver;          /// 流版本号
            bool   is_del_;             /// 是否被删除
        };
    } //namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_LOGSTREAM_H_
