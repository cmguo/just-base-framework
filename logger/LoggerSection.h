// LoggerSection.h

#ifndef _FRAMEWORK_LOGGER_SECTION_H_
#define _FRAMEWORK_LOGGER_SECTION_H_

#include "framework/logger/LoggerManager.h"

namespace framework
{
    namespace logger
    {

        class LoggerSection
        {
        public:
            LoggerSection(
                LoggerManager & loggermgr = global_logger_mgr())
                : loggermgr_(loggermgr)
                , started_(false)
            {
                start();
            }

            ~LoggerSection()
            {
                stop();
            }

            void start()
            {
                loggermgr_.log_sec_beg();
                started_ = true;
            }

            void stop()
            {
                if (started_) {
                    loggermgr_.log_sec_end();
                    started_ = false;
                }
            }

        private:
            LoggerManager & loggermgr_;
            bool started_;
        };

    } // namespace logger
} // namespace framework

#define LOG_SECTION() \
    framework::logger::LoggerSection _log_sec

#endif // _FRAMEWORK_LOGGER_SECTION_H_
