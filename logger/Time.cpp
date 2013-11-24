// Time.cpp

#include "framework/Framework.h"
#include "framework/logger/Time.h"
#include "framework/logger/Message.h"
#include "framework/logger/Buffer.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#define DATE_START  1
#define TIME_START  (DATE_START + 11)

namespace framework
{
    namespace logger
    {

        /// 获取当前时间串
        ///
        ///     获取当前的时间，转换为指定的字符串格式
        ///     @param buf 用以接收格式化后时间的字符串
        ///     @param size 缓冲大小
        ///     @param fmt 时间格式
        ///     @return 格式化后的时间串
        static char const *time_str_now(
            char * buf, 
            size_t size, 
            char const * fmt)
        {
            boost::posix_time::ptime t = 
                boost::posix_time::second_clock::local_time();
            typedef boost::date_time::time_facet<boost::posix_time::ptime, char> facet_t;;
            Buffer buffer(buf, size);
            std::ostream os(&buffer);
            os.imbue(std::locale(os.getloc(),new facet_t(fmt)));
            os << t;
            os.put(0);
            return buf;
        }

        Time::Time(
            bool is_file)
            : is_file_(is_file)
        {
            time_str_now(time_str_, sizeof(time_str_), is_file_ ? file_time_format_str : time_format_str);
            time_t tt = time(NULL);
            boost::posix_time::ptime t = boost::posix_time::from_time_t(tt);
            typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adjustor_t;
            boost::posix_time::ptime t1 = local_adjustor_t::utc_to_local(t);
            boost::posix_time::ptime t2(t1.date());
            mid_night_ = tt - (t1 - t2).total_seconds();
            update();
        }

        bool Time::check() const
        {
            int t_diff = (int)(time(NULL) - mid_night_);
            return t_diff >= 24 * 60 * 60;  
        }

        void Time::update()
        {
            int t_diff = (int)(time(NULL) - mid_night_);
            if (t_diff >= 24 * 60 * 60) {
                time_str_now(time_str_, sizeof(time_str_), is_file_ ? file_time_format_str : time_format_str);
                mid_night_ += 24 * 60 * 60;
                t_diff -= 24 * 60 * 60;
            }
        }

        void Time::update(
            boost::mutex & lock)
        {
            int t_diff = (int)(time(NULL) - mid_night_);
            if (t_diff >= 24 * 60 * 60) {
                t_diff -= 24 * 60 * 60;
                // 双保险
                boost::mutex::scoped_lock locker(lock);
                update();
            }

            int hour = t_diff / 3600;
            int minute = t_diff % 3600;
            int second = minute % 60;
            minute /= 60;
            time_str_[TIME_START] = hour / 10 + '0';
            time_str_[TIME_START + 1] = hour % 10 + '0';
            time_str_[TIME_START + 3] = minute / 10 + '0';
            time_str_[TIME_START + 4] = minute % 10 + '0';
            time_str_[TIME_START + 6] = second / 10 + '0';
            time_str_[TIME_START + 7] = second % 10 + '0';
        }

    } // namespace logger
} // namespace framework
