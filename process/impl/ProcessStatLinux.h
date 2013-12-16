// Process.cpp

#include "framework/string/Format.h"

#include <boost/filesystem/path.hpp>
using namespace boost::filesystem;

#include <fstream>

namespace framework
{
    namespace process
    {

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

        boost::system::error_code get_system_stat(
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
            return boost::system::error_code();
        }

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat)
        {
            path ph(path("/proc") / framework::string::format(pid) / "stat");
            std::ifstream ifs(ph.file_string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            ifs >> stat;
            return boost::system::error_code();
        }

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm)
        {
            path ph(path("/proc") / framework::string::format(pid) / "statm");
            std::ifstream ifs(ph.file_string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            ifs >> statm;
            return boost::system::error_code();
        }

    } // namespace process
} // namespace framework
