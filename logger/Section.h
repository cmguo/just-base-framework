// Section.h

#ifndef _FRAMEWORK_LOGGER_SECTION_H_
#define _FRAMEWORK_LOGGER_SECTION_H_

#include "framework/logger/Logger.h"

namespace framework
{
    namespace logger
    {

        class Section
        {
        public:
            Section(
                Manager & loggermgr = _slog())
                : manager_(loggermgr)
                , started_(false)
            {
                start();
            }

            ~Section()
            {
                stop();
            }

            void start()
            {
                //manager_.sec_beg(*this);
                started_ = true;
            }

            void stop()
            {
                if (started_) {
                    //manager_.sec_end();
                    started_ = false;
                }
            }

        private:
            Manager & manager_;
            bool started_;
        };

    } // namespace logger
} // namespace framework

#define LOG_SECTION() \
    framework::logger::Section _log_sec;

#endif // _FRAMEWORK_LOGGER_SECTION_H_
