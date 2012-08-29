// Time.h

#ifndef _FRAMEWORK_LOGGER_TIME_H_
#define _FRAMEWORK_LOGGER_TIME_H_

#include <boost/thread/mutex.hpp>

namespace framework
{
    namespace logger
    {

        class Time
        {
        public:
            Time();

            bool check() const;

            void update(
                boost::mutex & lock);

            void update();

            char const * time_str() const
            {
                return time_str_;
            }

        private:
            char time_str_[32];                     /// ÈÕÆÚ´®
            time_t mid_night_;
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_TIME_H_
