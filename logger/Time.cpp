// Time.cpp

#include "framework/Framework.h"
#include "framework/logger/Time.h"
#include "framework/logger/Message.h"

#ifdef BOOST_WINDOWS_API
#  if (defined UNDER_CE) || (defined __MINGW32__)
#    define localtime_r(x, y) *y = *localtime(x)
#  else 
#    define localtime_r(x, y) localtime_s(y, x)
#  endif
#  define snprintf _snprintf
#  define mkdir(x, y) mkdir(x)
#endif

#define DATE_START  1
#define TIME_START  (DATE_START + 11)

namespace framework
{
    namespace logger
    {

        /// ��ȡ��ǰʱ�䴮
        ///
        ///     ��ȡ��ǰ��ʱ�䣬ת��Ϊָ�����ַ�����ʽ
        ///     @param buf ���Խ��ո�ʽ����ʱ����ַ���
        ///     @param size �����С
        ///     @param fmt ʱ���ʽ
        ///     @return ��ʽ�����ʱ�䴮
        static char const *time_str_now(
            char * buf, 
            size_t size, 
            char const * fmt)
        {
            time_t tt = time(NULL);
            struct tm lt;
            localtime_r(&tt, &lt);
            strftime(buf, size, fmt, &lt);
            return buf;
        }

        Time::Time()
        {
            time_str_now(time_str_, sizeof(time_str_), time_format_str);
            time_t tt = time(NULL);
            struct tm lt;
            localtime_r(&tt, &lt);
            lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
            mid_night_ = mktime(&lt);
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
                time_str_now(time_str_, sizeof(time_str_), time_format_str);
                mid_night_ += 24 * 60 * 60;
                t_diff -= 24 * 60 * 60;
            }
        }

        void Time::update(
            boost::mutex & lock)
        {
            int t_diff = (int)(time(NULL) - mid_night_);
            if (t_diff >= 24 * 60 * 60) {
                // ˫����
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
