// TimeHelper.h

#ifndef _FRAMEWORK_TIMER_TIME_HELPER_H_
#define _FRAMEWORK_TIMER_TIME_HELPER_H_

#include <time.h>

namespace framework
{
    namespace timer
    {

        struct TimeHelper
        {
            typedef struct tm tm_t;

            static time_t now();

            static std::string local_time_str(
                char const * fmt);

            static std::string utc_time_str(
                char const * fmt);

            static std::string local_time_str(
                char const * fmt, 
                time_t t);

            static std::string utc_time_str(
                char const * fmt, 
                time_t t);

            static time_t make_local_time(
                tm_t * tm);

            static time_t make_utc_time(
                tm_t * tm);
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_TIME_HELPER_H_
