// FormatRecord.h

#ifndef _FRAMEWORK_LOGGER_FORMAT_RECORD_H_
#define _FRAMEWORK_LOGGER_FORMAT_RECORD_H_

#include "framework/logger/Record.hpp"

namespace framework
{
    namespace logger
    {

        class FormatFormator
        {
        public:
            FormatFormator(
                std::streambuf * buf)
                : os_(buf)
                , fmt_(NULL)
            {
            }

            ~FormatFormator()
            {
                os_.write(fmt_, strlen(fmt_));
            }

            void operator()(
                char const * t)
            {
                if (fmt_) {
                    format(t);
                } else {
                    fmt_ = t;
                }
            }

            template <
                typename _Ty
            >
            void operator()(
                _Ty const & t)
            {
                format(t);
            }

        private:
            template <
                typename _Ty
            >
            void format(
                _Ty const & t)
            {
                next_format();
                os_ << t;
            }

            void next_format();

        private:
            std::ostream os_;
            char const * fmt_;   /// ¸ñÊ½»¯´®
        };

    } // namespace logger
} // namespace framework

#define LOG_F(level, params) LOG_(level, framework::logger::FormatFormator, %, params)

#ifdef LOG
#  undef LOG
#endif
#define LOG LOG_F

#include "framework/logger/Macro.h"

#endif // _FRAMEWORK_LOGGER_FORMAT_RECORD_H_
