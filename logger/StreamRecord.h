// StreamRecord.h

#ifndef _FRAMEWORK_LOGGER_STREAM_RECORD_H_
#define _FRAMEWORK_LOGGER_STREAM_RECORD_H_

#include "framework/logger/Record.hpp"

namespace framework
{
    namespace logger
    {

        class StreamFormator
        {
        public:
            StreamFormator(
                std::streambuf * buf)
                : os_(buf)
            {
            }

            template <
                typename _Ty
            >
            void operator()(
                _Ty const & t)
            {
                os_ << t;
            }

        private:
            std::ostream os_;
        };

    } // namespace logger
} // namespace framework

#define LOG_S(level, params) LOG_(level, framework::logger::StreamFormator, <<, params)

#ifdef LOG
#  undef LOG
#endif
#define LOG LOG_S

#include "framework/logger/Macro.h"

#endif // _FRAMEWORK_LOGGER_STREAM_RECORD_H_
