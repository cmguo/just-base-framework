// ProcessEnumLinux.h

#include "framework/filesystem/Symlink.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"

#if defined(__APPLE__)
#  include <sys/sysctl.h>
#  define MAX_PROCESS_RANGE             102400
#  define MAX_PATH_LENGTH               2048
#endif

namespace framework
{
    namespace process
    {

        bool has_process_name(
            path const & bin_file, 
            path const & bin_path)
        {
            return bin_file.empty() 
                || (bin_file.is_complete() 
                    ? bin_path == bin_file 
                    : bin_path.leaf() == bin_file.leaf());
        }

#ifdef __FreeBSD__
#  define PROC_EXE "file"
#else
#  define PROC_EXE "exe"
#endif

        bool get_process_info(
            ProcessInfo & info, 
            int pid, 
            path const & bin_file, 
            error_code & ec)
        {
            path ph(path("/proc") / framework::string::format(pid));
            if (!is_symlink(ph) // ¹ýÂË/proc/self
                && is_symlink(ph / PROC_EXE)) {
                    info.bin_file = framework::filesystem::read_symlink(ph / PROC_EXE, ec);
                    if (!ec && has_process_name(bin_file, info.bin_file)) {
                        info.pid = (int)(pid);
                        return true;
                    }
            } else {
                ec = error::not_open;
            }
            return false;
        }


#if defined (__APPLE__)

        boost::system::error_code enum_process(
            path const & bin_file, 
            std::vector<ProcessInfo> & processes)
        {
            int mib[4];
            size_t i, len;
            struct kinfo_proc kp;

            len = 4;
            if (sysctlnametomib("kern.proc.pid", mib, &len) < 0) {
                return last_system_error();
            }

            for (i = 0; i < MAX_PROCESS_RANGE; i++) {
                mib[3] = i;
                len = sizeof(kp);
                if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1) {
                    return last_system_error();
                } else if (len > 0) {
                    int mib1[4];
                    char *proc_path = NULL;
                    size_t size = MAX_PATH_LENGTH;

                    mib1[0] = CTL_KERN;
                    mib1[1] = KERN_PROCARGS;
                    mib1[2] = kp.kp_proc.p_pid;
                    mib1[3] = 0;

                    proc_path = new char[size * sizeof(char)];
                    if (sysctl(mib1, 3, proc_path, &size, NULL, 0) < 0) {
                        continue;
                    } else {
                        ProcessInfo p_info;
                        p_info.bin_file = proc_path;
                        if (p_info.bin_file.leaf() == bin_file || bin_file.empty()) {
                            p_info.pid =  kp.kp_proc.p_pid;
                            processes.push_back(p_info);
                        }
                    }
                    delete[] proc_path;
                }
            }
            return error_code();
        }

#else

        boost::system::error_code enum_process(
            path const & bin_file, 
            std::vector<ProcessInfo> & processes)
        {
            path proc_path("/proc");
            if (!exists(proc_path))
                return framework::system::logic_error::not_supported;
            directory_iterator iter(proc_path);
            directory_iterator end;
            for (; iter != end; ++iter) {
                ProcessInfo info;
                error_code ec;
                if (get_process_info(info, framework::string::parse<pid_t>(iter->path().filename().string()), bin_file, ec)) {
                    processes.push_back(info);
                }
            }
            return error_code();
        }

#endif

    } // namespace process

    namespace this_process
    {

        int id()
        {
            return (int)::getpid();
        }

        int parent_id()
        {
            return (int)::getppid();
        }

    } // namespace this_process
} // namespace framework
