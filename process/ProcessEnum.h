// ProcessEnum.h

#ifndef _FRAMEWORK_PROCESS_PROCESS_ENUM_H_
#define _FRAMEWORK_PROCESS_PROCESS_ENUM_H_

#include <boost/filesystem/path.hpp>

namespace framework
{
    namespace process
    {

        struct ProcessInfo
        {
            int pid;
            boost::filesystem::path bin_file;
        };

        bool has_process_name(
            boost::filesystem::path const & bin_file, 
            boost::filesystem::path const & bin_path);

        bool get_process_info(
            ProcessInfo & info, 
            int pid, 
            boost::filesystem::path const & bin_file, 
            boost::system::error_code & ec);

        boost::system::error_code enum_process(
            boost::filesystem::path const & bin_file, 
            std::vector<ProcessInfo> & processes);

        inline boost::system::error_code enum_process(
            std::vector<ProcessInfo> & processes)
        {
            return enum_process(boost::filesystem::path(), processes);
        }

		class Process;

		Process & self();

    } // namespace process

    namespace this_process
    {

        int id();

        int parent_id();

    } // namespace this_process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_PROCESS_ENUM_H_
