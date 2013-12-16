// ProcessStatWin32.h

#include <windows.h>
#if (!defined UNDER_CE) && (!defined WINRT) && (!defined WIN_PHONE)
#  include <psapi.h>
#  pragma comment(lib, "Psapi.lib")
#endif

namespace framework
{
    namespace process
    {

#if (!defined UNDER_CE) && (!defined WINRT) && (!defined WIN_PHONE)

        boost::system::error_code get_system_stat(
            SystemStat & stat)
        {
            FILETIME t[3];
            BOOL ret = ::GetSystemTimes(t, t + 1, t + 2);
            if (ret) {
                stat.cpu.user = t[2].dwLowDateTime;
                stat.cpu.system = t[1].dwLowDateTime;
                stat.cpu.idle = t[0].dwLowDateTime;
                stat.cpu.total = stat.cpu.idle + stat.cpu.system + stat.cpu.user;
            }
            return last_system_error();
        }

        boost::system::error_code get_process_stat(
            HANDLE hp, 
            ProcessStat & stat)
        {
            FILETIME t[4];
            BOOL ret = ::GetProcessTimes(
                hp, 
                t, 
                t + 1, 
                t + 2, 
                t + 3);
            if (ret) {
                stat.pid = ::GetProcessId(hp);
                stat.state = ProcessStat::running;
                stat.stime = t[2].dwLowDateTime;
                stat.utime = t[3].dwLowDateTime;
            }
            return last_system_error();
        }

        boost::system::error_code get_process_statm(
            HANDLE hp, 
            ProcessStatM & statm)
        {
            PROCESS_MEMORY_COUNTERS pmc = {sizeof(pmc)};
            if (::GetProcessMemoryInfo(
                hp, 
                &pmc, 
                sizeof(pmc))) {
                statm.size = pmc.PagefileUsage;
                statm.resident = pmc.WorkingSetSize;
            }
            return last_system_error();
        }

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat)
        {
            HANDLE hProcess = ::OpenProcess(
                PROCESS_ALL_ACCESS, 
                FALSE, 
                pid);
            if (hProcess == NULL) {
                return last_system_error();
            }
            return get_process_stat(hProcess, stat);
        }

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm)
        {
            HANDLE hProcess = ::OpenProcess(
                PROCESS_ALL_ACCESS, 
                FALSE, 
                pid);
            if (hProcess == NULL) {
                return last_system_error();
            }
            return get_process_statm(hProcess, statm);
        }

#else

        boost::system::error_code get_system_stat(
            SystemStat & stat)
        {
            memset(&stat.cpu, 0, sizeof(stat.cpu));
            SetLastError(ERROR_NOT_SUPPORTED);
            return last_system_error();
        }

        boost::system::error_code get_process_stat(
            HANDLE hp, 
            ProcessStat & stat)
        {
            memset(&stat, 0, sizeof(stat));
            SetLastError(ERROR_NOT_SUPPORTED);
            return last_system_error();
        }

        boost::system::error_code get_process_statm(
            HANDLE hp, 
            ProcessStatM & statm)
        {
            memset(&statm, 0, sizeof(statm));
            SetLastError(ERROR_NOT_SUPPORTED);
            return last_system_error();
        }

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat)
        {
            return get_process_stat((HANDLE)NULL, stat);
        }

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm)
        {
            return get_process_statm((HANDLE)NULL, statm);
        }


#endif

    } // namespace process
} // namespace framework
