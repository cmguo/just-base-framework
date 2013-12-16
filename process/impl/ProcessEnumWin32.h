// ProcessEnumWin32.h

#include "framework/filesystem/Symlink.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
using namespace framework::system;
using namespace framework::string;

#include <boost/filesystem/operations.hpp>
using namespace boost::filesystem;
using namespace boost::system;

#include <windows.h>

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
                : bin_path.stem() == bin_file.stem());
        }

#if (defined UNDER_CE) || (!defined WINRT) || (!defined WIN_PHONE)

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

#else

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

#endif

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
