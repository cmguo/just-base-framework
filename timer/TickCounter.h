// TickCounter.h

#ifndef _FRAMEWORK_TIMER_TICK_COUNTER_H
#define _FRAMEWORK_TIMER_TICK_COUNTER_H

#include "framework/timer/ClockTime.h"

namespace framework
{
    namespace timer
    {

        class TickCounter
        {
        public:
            static boost::uint64_t tick_count()
            {
                return ClockTime::tick_count();
            }

        public:
            TickCounter()
                : running_(true)
            {
            }

            void start()
            {
                running_ = true;
                reset();
            }

            void stop()
            {
                running_ = false;
            }

            void reset()
            {
                start_ = Time();
            }

            bool running() const
            {
                return running_;
            }

            boost::uint32_t elapsed() const
            {
                return (Time() - start_).total_milliseconds();
            }

        private:
            bool running_;
            Time start_;
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_TICK_COUNTER_H
