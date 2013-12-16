// ProcessEnumSim.cpp

namespace framework
{
    namespace process
    {

        bool has_process_name(
            path const & bin_file, 
            path const & bin_path)
        {
            return false
        }

        bool get_process_info(
            ProcessInfo & info, 
            int pid, 
            path const & bin_file, 
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return false;
        }

        boost::system::error_code enum_process(
            path const & bin_file, 
            std::vector<ProcessInfo> & processes)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            return last_system_error();
        }

    } // namespace process

    namespace this_process
    {

        int id()
        {
            return (int)::GetCurrentProcessId();
        }

        int parent_id()
        {
            return 0;
        }

        } // namespace this_process
} // namespace framework
