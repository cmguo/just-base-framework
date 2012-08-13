// TimeConvert.cpp

#include "framework/Framework.h"
#include "framework/timer/TimeConvert.h"

#if (defined(__ANDROID__)) && (!defined(__SONY_ANDROID_TV__))
#include <time64.h>
#endif

namespace framework
{
    namespace timer
    {
        time_t time_gm( struct tm * tm )
        {
#ifdef __FreeBSD__
            return timegm( tm );
#elif (defined(__ANDROID__)) && (!defined(__SONY_ANDROID_TV__))
            return ( time_t )( timegm64( tm ) );
#else
            return ( mktime( tm ) - ( time_t )timezone );
#endif
        }
    } // namespace timer
} // namespace framework
