// ProcessEnum.cpp

#include "framework/Framework.h"
#include "framework/system/ErrorCode.h"
#include "framework/process/ProcessEnum.h"
#include "framework/process/Process.h"
#include "framework/process/Error.h"
#include "framework/filesystem/Symlink.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
using namespace framework::system;
using namespace framework::string;

#include <boost/filesystem/operations.hpp>
using namespace boost::filesystem;
using namespace boost::system;

#include <fstream>

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#  ifndef __MINGW32__
#    include <winternl.h>
#  endif
#  ifdef UNDER_CE
#    ifndef STARTF_USESTDHANDLES
#       define STARTF_USESTDHANDLES     0x00000100
#    endif
#    ifndef NORMAL_PRIORITY_CLASS
#       define NORMAL_PRIORITY_CLASS    0x00000020
#    endif
#    ifndef CREATE_NO_WINDOW
#       define CREATE_NO_WINDOW         0x08000000
#    endif
#  else
#    include <fcntl.h>
#    include <io.h>
#    include <psapi.h>
#    pragma comment(lib, "Psapi.lib")
#  endif
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <signal.h>
#  include <poll.h>
#if defined(__APPLE__)
#  include <sys/sysctl.h>
#  define MAX_PROCESS_RANGE             102400
#  define MAX_PATH_LENGTH               2048
#endif
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
#ifndef BOOST_WINDOWS_API
                    : bin_path.leaf() == bin_file.leaf());
#else
                    : bin_path.stem() == bin_file.stem());
#endif
        }

#ifndef BOOST_WINDOWS_API

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
            path ph(path("/proc") / format(pid));
            if (!is_symlink(ph) // 过滤/proc/self
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

#elif (!defined UNDER_CE) && (!defined WINRT)

        bool get_process_info(
            ProcessInfo & info, 
            int pid, 
            path const & bin_file, 
            error_code & ec)
        {
            CHAR szName[MAX_PATH] = "";
            HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, pid);
            if (hProcess) {
                HMODULE hModule;
                DWORD needed;
                // 枚举当前进程调用的所有模块
                if (::EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed)) {
                    ::GetModuleBaseName(hProcess, hModule, szName, sizeof(szName));
                    info.bin_file = szName;
                    if (has_process_name(bin_file, info.bin_file)) {
                        info.pid = (int)pid;
                        ::CloseHandle(hProcess);
                        return true;
                    }
                }
                ec = last_system_error();
                ::CloseHandle(hProcess);
                return false;
            }
            ec = last_system_error();
            return false;
        }

#else

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

#endif

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

#elif (!defined BOOST_WINDOWS_API)

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
                if (get_process_info(info, parse<pid_t>(iter->path().filename()), bin_file, ec)) {
                    processes.push_back(info);
                }
            }
            return error_code();
        }

#elif (!defined UNDER_CE) && (!defined WINRT)

        boost::system::error_code enum_process(
            path const & bin_file, 
            std::vector<ProcessInfo> & processes)
        {
            DWORD pids[1024], needed;
            if (!::EnumProcesses(pids, sizeof(pids), &needed)) {
                return last_system_error();
            }
            DWORD actualProcessCount = needed / sizeof(DWORD);
            for (DWORD i = 0; i < actualProcessCount; i++) {
                ProcessInfo info;
                error_code ec;
                if (get_process_info(info, pids[i], bin_file, ec)) {
                    processes.push_back(info);
                }
            }
            return error_code();
        }

#else
        boost::system::error_code enum_process(
            path const & bin_file, 
            std::vector<ProcessInfo> & processes)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            return last_system_error();
        }

#endif

		Process & self()
		{
			static Process p;
			if (!p.is_open()) {
				error_code ec;
				p.open(this_process::id(), ec);
				assert(!ec);
			}
			return p;
		}

    } // namespace process

    namespace this_process
    {

        int id()
        {
#ifndef BOOST_WINDOWS_API
            return (int)::getpid();
#else
            return (int)::GetCurrentProcessId();
#endif
        }

        int parent_id()
        {
#ifndef BOOST_WINDOWS_API
            return (int)::getppid();
#else
            return 0;
#endif
        }

    } // namespace this_process
} // namespace framework
