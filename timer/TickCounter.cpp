// TickCounter.cpp

#include "framework/Framework.h"
#include "framework/timer/TickCounter.h"

#ifdef BOOST_WINDOWS_API
#  include <Windows.h>
#  pragma comment(lib, "Kernel32.lib")
#else
#  include <sys/time.h>
//#  include <time.h>
//#  ifndef CLOCK_MONOTONIC
//#    define CLOCK_MONOTONIC CLOCK_REALTIME
//#  endif
#include <unistd.h>
#endif

namespace framework
{
    namespace timer
    {

        boost::uint64_t TickCounter::tick_count()
        {
#ifdef BOOST_WINDOWS_API
            return ::GetTickCount();
#else
            struct timespec t = { 0 };
            int res = clock_gettime(CLOCK_MONOTONIC, &t);
            assert(0 == res);
            boost::uint64_t val = t.tv_sec * 1000 + t.tv_nsec / 1000 / 1000;
            return val;
#endif
        }

    } // namespace timer
} // namespace framework
