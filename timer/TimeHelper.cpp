// TimeHelper.cpp

#include "framework/Framework.h"
#include "framework/timer/TimeHelper.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <sstream>

//#if (defined(__ANDROID__)) && (!defined(__SONY_ANDROID_TV__))
//#include <time64.h>
//#endif

namespace framework
{
    namespace timer
    {


        time_t TimeHelper::now()
        {
            return time(NULL);
        }

        std::string TimeHelper::local_time_str(
            char const * fmt)
        {
            return local_time_str(fmt, now());
        }

        std::string TimeHelper::utc_time_str(
            char const * fmt)
        {
            return utc_time_str(fmt, now());
        }

        static std::string time_str(
            char const * fmt, 
            boost::posix_time::ptime pt)
        {
            typedef boost::date_time::time_facet<boost::posix_time::ptime, char> facet_t;
            std::ostringstream oss;
            oss.imbue(std::locale(oss.getloc(),new facet_t(fmt)));
            oss << pt;
            return oss.str();
        }

        std::string TimeHelper::local_time_str(
            char const * fmt, 
            time_t t)
        {
            typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adjustor_t;
            boost::posix_time::ptime pt = boost::posix_time::from_time_t(t);
            boost::posix_time::ptime pt1 = local_adjustor_t::utc_to_local(pt);
            return time_str(fmt, pt1);
        }

        std::string TimeHelper::utc_time_str(
            char const * fmt, 
            time_t t)
        {
            typedef boost::date_time::time_facet<boost::posix_time::ptime, char> facet_t;
            boost::posix_time::ptime pt = boost::posix_time::from_time_t(t);
            return time_str(fmt, pt);
        }

        time_t TimeHelper::make_local_time(
            tm_t * tm)
        {
            //typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adjustor_t;
            //boost::posix_time::ptime start(boost::gregorian::date(1970,1,1));
            //boost::posix_time::ptime pt = boost::posix_time::ptime_from_tm(*tm);
            //boost::posix_time::ptime pt1 = local_adjustor_t::utc_to_local(pt);
            //time_t t = (pt1 - start).total_seconds();
            return mktime(tm);
        }

        time_t TimeHelper::make_utc_time(
            tm_t * tm)
        {
            boost::posix_time::ptime start(boost::gregorian::date(1970,1,1));
            boost::posix_time::ptime pt = boost::posix_time::ptime_from_tm(*tm);
            time_t t = (time_t)(pt - start).total_seconds();
            return t;
//#ifdef __FreeBSD__
//            return timegm(tm);
//#elif (defined(__ANDROID__)) && (!defined(__SONY_ANDROID_TV__))
//            return (time_t)(timegm64(tm));
//#else
//            return mktime(tm) - (time_t)timezone;
//#endif
        }

    } // namespace timer
} // namespace framework
