// Process.h

#ifndef _FRAMEWORK_PROCESS_PROCESS_H_
#define _FRAMEWORK_PROCESS_PROCESS_H_

#include "framework/process/ProcessStat.h"
#include "framework/process/ProcessEnum.h"
#include "framework/process/SignalHandler.h"

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace framework
{
    namespace process
    {

        namespace detail
        {
            struct process_data_base;
        }

        struct ProcessStat;
        struct ProcessStatM;

        class Process
            : private boost::noncopyable
        {
        public:
            Process();

            ~Process();

            struct CreateParamter
            {
                CreateParamter()
                    : outfd(-1)
                    , infd(-1)
                    , errfd(-1)
                    , wait(false)
                {
                }

                std::vector<std::string> args;
                int outfd;
                int infd;
                int errfd;
                bool wait;
            };

            boost::system::error_code open(
                boost::filesystem::path const & bin_file, 
                CreateParamter const & paramter, 
                boost::system::error_code & ec);

            boost::system::error_code create(
                boost::filesystem::path const & bin_file, 
                CreateParamter const & paramter, 
                boost::system::error_code & ec);

            boost::system::error_code create(
                boost::filesystem::path const & bin_file, 
                boost::system::error_code & ec);

            boost::system::error_code open(
                boost::filesystem::path const & bin_file, 
                boost::system::error_code & ec);

            boost::system::error_code open(
                int pid, 
                boost::system::error_code & ec);

            bool is_open();

            bool is_alive(
                boost::system::error_code & ec);

            boost::system::error_code join(
                boost::system::error_code & ec);

            boost::system::error_code timed_join(
                unsigned long milliseconds, 
                boost::system::error_code & ec);

            /**
            Send signal to this process.
            It don't work on win32 os
            */
            boost::system::error_code signal(
                Signal sig, 
                boost::system::error_code & ec);

            boost::system::error_code detach(
                boost::system::error_code & ec);

            boost::system::error_code kill(
                boost::system::error_code & ec);

            int exit_code(
                boost::system::error_code & ec);

            boost::system::error_code close(
                boost::system::error_code & ec);

            boost::system::error_code stat(
                ProcessStat & stat) const;

            boost::system::error_code statm(
                ProcessStatM & statm) const;

        public:
            int id() const;

            int parent_id() const;

        public:
            void swap(
                Process & x)
            {
                std::swap(data_, x.data_);
            }

        private:
            detail::process_data_base * data_;
        };

        Process & self();

        bool notify_wait(
            boost::system::error_code const & ec);

    } // namespace process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_PROCESS_H_
