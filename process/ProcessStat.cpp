// Process.cpp

#include "framework/Framework.h"
#include "framework/system/ErrorCode.h"
#include "framework/process/ProcessStat.h"
using namespace framework::system;

#include <fstream>

using namespace boost::system;

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

#ifndef BOOST_WINDOWS_API

#ifdef __FreeBSD__
#  define PROC_EXE "file"
#else
#  define PROC_EXE "exe"
#endif

		std::istream & operator>>(
            std::istream & is, 
            CpuStat & stat)
        {
            is >> stat.user
                >> stat.nice
                >> stat.system
                >> stat.idle
                >> stat.iowait
                >> stat.irq
                >> stat.softirq;
            stat.total = stat.user + stat.nice + stat.system
                + stat.idle + stat.iowait + stat.irq + stat.softirq;
            return is;
        }

        ProcessStat::StateEnum const ProcessStat::char_to_state[] = {
            dead, dead, dead, sleep1, dead, dead, dead, // A - G
            dead, dead, dead, dead, dead, dead, dead,   // H - N
            dead, dead, dead, running, sleep2, stopped, // O - T
            dead, dead, dead, dead, dead, zombie,       // U - Z
        };

        std::istream & operator>>(
            std::istream & is, 
            ProcessStat & stat)
        {
            is >> stat.pid;
            is.ignore(1);
            is >> stat.name;
            char state;
            is >> state;
            stat.state = ProcessStat::char_to_state[state - 'A'];
            is >> stat.ppid >> stat.pgid >> stat.sid
                >> stat.tty_nr >> stat.tty_pgrp
                >> stat.flags
                >> stat.min_flt >> stat.cmin_flt >> stat.maj_flt >> stat.cmaj_flt
                >> stat.utime >> stat.stime >> stat.cutime >> stat.cstime
                >> stat.priority >> stat.nice
                >> stat.num_threads
                >> stat.it_real_value
                >> stat.start_time
                >> stat.vsize
                >> stat.rss
                >> stat.rlim
                >> stat.start_code >> stat.end_code
                >> stat.start_stack
                >> stat.kstkesp >> stat.kstkeip
                >> stat.pendingsig >> stat.block_sig >> stat.sigign >> stat.sigcatch
                >> stat.wchan
                >> stat.nswap
                >> stat.cnswap
                >> stat.exit_signal
                >> stat.task_cpu >> stat.task_priority >> stat.task_policy;
            return is;
        }

        std::istream & operator>>(
            std::istream & is, 
            ProcessStatM & statm)
        {
            is >> statm.size
                >> statm.resident
                >> statm.shared
                >> statm.trs
                >> statm.lrs
                >> statm.drs
                >> statm.dt;
            return is;
        }

        error_code get_system_stat(
            SystemStat & stat)
        {
            path ph("/proc/stat");
            std::ifstream ifs(ph.file_string().c_str());
            std::string title;
            ifs >> title >> stat.cpu;
            while (ifs >> title) {
                if (title.substr(0, 3) == "cpu") {
                    stat.cpus.resize(stat.cpus.size() + 1);
                    ifs >> stat.cpus.back();
                } else {
                    break;
                }
            }
            return error_code();
        }

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat)
        {
            path ph(path("/proc") / format(pid) / "stat");
            std::ifstream ifs(ph.file_string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            ifs >> stat;
            return error_code();
        }

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm)
        {
            path ph(path("/proc") / format(pid) / "statm");
            std::ifstream ifs(ph.file_string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            ifs >> statm;
            return error_code();
        }

#elif (!defined UNDER_CE) && (!defined WINRT)

        error_code get_system_stat(
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

        error_code get_system_stat(
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
