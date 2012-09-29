// Message.h

#ifndef _FRAMEWORK_LOGGER_MESSAGE_H_
#define _FRAMEWORK_LOGGER_MESSAGE_H_

#include "framework/logger/Level.h"

namespace framework
{
    namespace logger
    {

        enum MessageItemEnum
        {
#ifndef BOOST_WINDOWS_API
            mi_color, 
#endif
            mi_time, 
            mi_pid, 
            mi_level, 
            mi_module, 
            mi_msg, 
#ifndef BOOST_WINDOWS_API
            mi_color2, 
#endif
            mi_other, 
            mi_max, 
        };

        /// ´íÎóµÈ¼¶×Ö·û´®
        static char const * const level_str[] = {
            "[Off..] ", 
            "[FATAL] ", 
            "[ERROR] ", 
            "[WARN.] ", 
            "[INFO.] ", 
            "[DEBUG] ", 
            "[TRACE] ", 
            "[All..] "
        };

        static char const * const color_str[] = {
            "\033[1;0m", 
            "\033[0m", 
        };

        static LevelEnum str_to_level(
            char c)
        {
            static LevelEnum level[] = {
                All, None, None, Debug, Error, Fatal, None, 
                None, Info, None, None, None, None, None, 
                Off, None, None, None, None, Trace, 
                None, None, Warn, None, None, None
            };
            return level[c - 'A'];
        }

        static const size_t level_str_len = 8;
        static const size_t color_str_len[] = {6, 4};

        static char const * const time_format_str = "<%Y-%m-%d %H:%M:%S> ";

        static const size_t time_str_len = 24;

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_MESSAGE_H_
