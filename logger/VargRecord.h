// VargRecord.h

#ifndef _FRAMEWORK_LOGGER_VARG_RECORD_H_
#define _FRAMEWORK_LOGGER_VARG_RECORD_H_
#include "framework/logger/Record.h"
#include "framework/logger/Buffer.h"

#include <stdarg.h>

#ifdef BOOST_WINDOWS_API
#  define va_copy(dst, src) dst = src
#endif

namespace framework
{
    namespace logger
    {

        class VargRecord
            : public Record
        {
        public:
            VargRecord(
                char const * fmt, 
                va_list args)
                : Record(format_message)
                , fmt_(fmt)
            {
                va_copy(args_, args);
            }

            VargRecord(
                char const * title, 
                char const * fmt, 
                va_list args)
                : Record(format_message)
                , fmt_(fmt)
            {
                va_copy(args_, args);
            }

        private:
            static size_t format_message(
                Record const & base, 
                char * buf, 
                size_t len)
            {
                VargRecord const & me = 
                    static_cast<VargRecord const &>(base);

                return vsnprintf(buf, len, me.fmt_, me.args_);
            }

        private:
            char const * fmt_;
            mutable va_list args_;
        };

    } // namespace logger
} // namespace framework

#define LOG_VARG(level, param) \
    if (framework::logger::log_accept(_slogm(), level)) \
        framework::logger::log(_slogm(), level, framework::logger::VargRecord param)

#endif // _FRAMEWORK_LOGGER_VARG_RECORD_H_
