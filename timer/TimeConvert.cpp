// TimeConvert.cpp

#include "framework/Framework.h"
#include "framework/timer/TimeConvert.h"

#ifdef __ANDROID__
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
#elif ( defined __ANDROID__ )
            return ( time_t )( timegm64( tm ) );
#else
            return ( mktime( tm ) - ( time_t )timezone );
#endif
        }
    } // namespace timer
} // namespace framework
