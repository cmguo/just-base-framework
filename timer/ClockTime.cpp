// ClockTime.cpp

#include "framework/Framework.h"
#include "framework/timer/ClockTime.h"

#ifdef BOOST_WINDOWS_API
#  include <Windows.h>
#else
#  include <time.h>
#  ifndef CLOCK_MONOTONIC
#    define CLOCK_MONOTONIC 1
#  endif
#include <unistd.h>
#endif

namespace framework
{
    namespace timer
    {

        Time Time::now()
        {
#ifdef BOOST_WINDOWS_API
            return Time(::GetTickCount());
#else
            struct timespec t = { 0 };
            int res = clock_gettime(CLOCK_MONOTONIC, &t);
            (void)res; // gcc warning
            assert(0 == res);
            boost::uint64_t val = t.tv_sec * 1000 + t.tv_nsec / 1000 / 1000;
            return Time(val);
#endif
        }

    } // namespace timer
} // namespace framework
