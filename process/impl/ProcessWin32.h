// Process.cpp

#include <windows.h>
#ifdef UNDER_CE
#  ifndef STARTF_USESTDHANDLES
#     define STARTF_USESTDHANDLES     0x00000100
#  endif
#  ifndef NORMAL_PRIORITY_CLASS
#     define NORMAL_PRIORITY_CLASS    0x00000020
#  endif
#  ifndef CREATE_NO_WINDOW
#     define CREATE_NO_WINDOW         0x08000000
#  endif
#else
#  include <io.h>
#endif

namespace framework
{
    namespace process
    {

        extern boost::system::error_code get_process_stat(
            HANDLE hp, 
            ProcessStat & stat);

        extern boost::system::error_code get_process_statm(
            HANDLE hp, 
            ProcessStatM & statm);

        namespace detail
        {
            struct process_data_base
            {
                bool is_alive;
                DWORD pid;
                DWORD ppid;
                HANDLE handle;
                DWORD status;

                process_data_base()
                    : is_alive(true)
                    , pid(0)
                    , handle(NULL)
                    , status(0)
                {
                }
            };
        }

        error_code Process::signal(
            Signal sig, 
            error_code & ec)
        {
            is_alive(ec);
            return ec;
        }

        error_code Process::stat(
            ProcessStat & stat) const
        {
            error_code ec;
            if (data_ && data_->is_alive) {
                ec = get_process_stat(data_->handle, stat);
            } else {
                ec = not_alive;
            }
            return ec;
        }

        error_code Process::statm(
            ProcessStatM & statm) const
        {
            error_code ec;
            if (data_ && data_->is_alive) {
                ec = get_process_statm(data_->handle, statm);
            } else {
                ec = not_alive;
            }
            return ec;
        }


#if (!defined WINRT) && (!defined WIN_PHONE)

        static HANDLE get_pipe_handle(int fd)
        {
#if (defined UNDER_CE)
            return INVALID_HANDLE_VALUE;
#else
            if (-1 == fd)
                return INVALID_HANDLE_VALUE;
            HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
            ::SetHandleInformation(h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            return h;
#endif
        }

        error_code Process::create(
            path const & bin_file, 
            CreateParamter const & paramter, 
            boost::system::error_code & ec)
        {
            if (is_open())
                return ec = already_open;

            std::string bin_file_path = bin_file.file_string();
            std::string cmdlinestr;
            if (bin_file_path.find(' ') == std::string::npos) {
                cmdlinestr = bin_file_path;
            } else {
                cmdlinestr = " \"" + bin_file_path + "\"";
            }
            for (size_t i = 0; i < paramter.args.size(); ++i) {
                if (!paramter.args[i].empty() && paramter.args[i].find(' ') == std::string::npos) {
                    cmdlinestr += " " + paramter.args[i];
                } else {
                    cmdlinestr += " \"" + paramter.args[i] + "\"";
                }
            }
            STARTUPINFO si;
            memset(&si, 0, sizeof(si));
            si.cb = sizeof( si );
            bool inheritHandles = false;
            if (-1 != paramter.outfd) {
                si.hStdOutput = get_pipe_handle(paramter.outfd);
                if (INVALID_HANDLE_VALUE != si.hStdOutput) {
                    inheritHandles = true;
                }
                si.dwFlags = STARTF_USESTDHANDLES;
            }
            if (-1 != paramter.infd) {
                si.hStdInput = get_pipe_handle(paramter.infd);
                if (INVALID_HANDLE_VALUE != si.hStdInput) {
                    inheritHandles = true;
                }
                si.dwFlags = STARTF_USESTDHANDLES;
            }
            if (-1 != paramter.errfd) {
                si.hStdError = get_pipe_handle(paramter.errfd);
                if (INVALID_HANDLE_VALUE != si.hStdError) {
                    inheritHandles = true;
                }
                si.dwFlags = STARTF_USESTDHANDLES;
            }
            {
                char pid_buf[32];
                size_t len = 0;
                len = framework::process::detail::format(pid_buf, GetCurrentProcessId());
                pid_buf[len] = '\0';
                ::SetEnvironmentVariable(
                    "FRAMEWORK_PROCESS_PARENT_ID", 
                    pid_buf);
            }
            HANDLE hReadPipe;
            HANDLE hWritePipe;
            if (paramter.wait) {
                ::CreatePipe(
                    &hReadPipe, 
                    &hWritePipe, 
                    NULL, 
                    0);
                char handle_buf[32];
                size_t len = 0;
                len = framework::process::detail::format(handle_buf, (boost::uint64_t)hWritePipe);
                assert((boost::uint64_t)hWritePipe == framework::process::detail::parse(handle_buf, len));
                handle_buf[len] = '\0';
                ::SetEnvironmentVariable(
                    "FRAMEWORK_PROCESS_WAIT_FILE", 
                    handle_buf);
            }
            PROCESS_INFORMATION pi = { 0 };
            if (FALSE == ::CreateProcess(
                NULL, 
                &cmdlinestr[0], 
                NULL, 
                NULL, 
                inheritHandles ? TRUE : FALSE, 
                NORMAL_PRIORITY_CLASS, 
                NULL, 
                NULL, 
                &si, 
                &pi)) {
                    ec = last_system_error();
                    if (paramter.wait) {
                        ::SetEnvironmentVariableA(
                            "FRAMEWORK_PROCESS_WAIT_FILE", 
                            NULL);
                        ::CloseHandle(
                            hReadPipe);
                        ::CloseHandle(
                            hWritePipe);
                    }
            } else {
                if (paramter.wait) {
                    ::SetEnvironmentVariableA(
                        "FRAMEWORK_PROCESS_WAIT_FILE", 
                        NULL);
                    CHAR Buffer[64] = {0};
                    DWORD NumberOfBytesRead = 0;
                    ::ReadFile(
                        hReadPipe, 
                        Buffer, 
                        sizeof(Buffer), 
                        &NumberOfBytesRead, 
                        NULL);
                    CHAR * p = (char *)::memchr(Buffer, ':', NumberOfBytesRead);
                    if (p) {
                        *p++ = '\0';
                        ec.assign((int)(detail::parse(p, Buffer + NumberOfBytesRead - p)), 
                            boost::system::error_category::find_category(Buffer));
                    } else {
                        ec = not_alive;
                    }
                    ::CloseHandle(
                        hWritePipe);
                    ::CloseHandle(
                        hReadPipe);
                }
                data_ = new detail::process_data_base;
                data_->pid = pi.dwProcessId;
                data_->ppid = ::GetCurrentProcessId();
                data_->handle = pi.hProcess;
                ::CloseHandle(
                    pi.hThread);
                ec = error_code();
            }
            LOG_DEBUG("create " 
                << bin_file.file_string() 
                << " " << (data_ ? data_->pid : 0) 
                << " " << ec.message());
            return ec;
        }

        error_code Process::open(
            path const & bin_file, 
            error_code & ec)
        {
            if (is_open())
                return ec = already_open;

            std::vector<ProcessInfo> pis;
            ec = enum_process(bin_file, pis);
            if (!ec) {
                if (pis.empty()) {
                    ec = not_open;
                } else {
                    open(pis[0].pid, ec);
                }
            }
            LOG_DEBUG("open " << bin_file.file_string() 
                << " " << (data_ ? data_->pid : 0) 
                << " " << ec.message());
            return ec;
        }

        error_code Process::open(
            int pid, 
            error_code & ec)
        {
            if (is_open())
                return ec = already_open;

            data_ = new detail::process_data_base;
            data_->pid = pid;
            data_->ppid = 0;
            data_->handle = ::OpenProcess(
                PROCESS_ALL_ACCESS, 
                FALSE, 
                pid);
            if (!data_->handle) {
                ec = last_system_error();
                delete data_;
                data_ = NULL;
            }
            return ec;
        }

        bool Process::is_alive(
            error_code & ec)
        {
            if (!is_open()) {
                ec = not_open;
                return false;
            }
            ec = error_code();
            if (data_->is_alive) {
                BOOL success = ::GetExitCodeProcess(data_->handle, &data_->status);
                assert(success);
                if (!success) {
                    ec = last_system_error();
                    return false;
                }
                data_->is_alive = data_->status == STILL_ACTIVE;
                if (!data_->is_alive) {
                    ::CloseHandle(data_->handle);
                    data_->handle = NULL;
                }
                return data_->is_alive;
            } else {
                return false;
            }
        }

        error_code Process::join(
            error_code & ec)
        {
            if (is_alive(ec)) {
                if (WAIT_OBJECT_0 == ::WaitForSingleObject(data_->handle, INFINITE)) {
                    is_alive(ec);
                } else {
                    ec = last_system_error();
                }
            }
            return ec;
        }

        error_code Process::timed_join(
            unsigned long milliseconds, 
            error_code & ec)
        {
            if (is_alive(ec)) {
                DWORD waitResult = ::WaitForSingleObject(data_->handle, milliseconds);
                if (WAIT_OBJECT_0 == waitResult) {
                    is_alive(ec);
                } else {
                    ec = last_system_error();
                }
            }
            return ec;
        }

        error_code Process::detach(
            error_code & ec)
        {
            if (!is_open()) {
                ec = not_open;
                return ec;
            }
            ::CloseHandle(data_->handle);
            delete data_;
            data_ = NULL;
            return ec = error_code();
        }

        error_code Process::kill(
            error_code & ec)
        {
            if (is_alive(ec)) {
                ::TerminateProcess(data_->handle, 99);
                ::CloseHandle(data_->handle);
                data_->handle = NULL;
                data_->status = 99;
                data_->is_alive = false;
                ec = last_system_error();
            }
            return ec;
        }

        int Process::exit_code(
            error_code & ec)
        {
            if (is_alive(ec)) {
                ec = still_alive;
                return -1;
            }
            ec = error_code();
            return data_->status;
        }

        bool notify_wait(
            error_code const & ec)
        {
            char Buffer[64];
            HANDLE hParent = INVALID_HANDLE_VALUE;
            if (::GetEnvironmentVariable(
                "FRAMEWORK_PROCESS_PARENT_ID", 
                Buffer, 
                sizeof(Buffer)) > 0) {
                    DWORD dwPId = (DWORD)detail::parse(Buffer, strlen(Buffer));
                    hParent = ::OpenProcess(
                        PROCESS_ALL_ACCESS, 
                        FALSE, 
                        dwPId);
            }
            if (::GetEnvironmentVariable(
                "FRAMEWORK_PROCESS_WAIT_FILE", 
                Buffer, 
                sizeof(Buffer)) > 0) {
                    ::SetEnvironmentVariable(
                        "FRAMEWORK_PROCESS_WAIT_FILE", 
                        NULL);
                    HANDLE hFileWrite = (HANDLE)detail::parse(Buffer, strlen(Buffer));
                    ::DuplicateHandle(
                        hParent, 
                        hFileWrite, 
                        GetCurrentProcess(), 
                        &hFileWrite, 
                        0, 
                        FALSE, 
                        DUPLICATE_SAME_ACCESS);
                    size_t len = strlen(ec.category().name());
                    memcpy(Buffer, ec.category().name(), len);
                    Buffer[len++] = ':';
                    len += detail::format(Buffer + len, ec.value());
                    DWORD NumberOfBytesWrite = 0;
                    ::WriteFile(
                        hFileWrite, 
                        Buffer, 
                        len, 
                        &NumberOfBytesWrite, 
                        NULL);
                    ::CloseHandle(hFileWrite);
                    return true;
            }
            return false;
        }

#else

        error_code Process::create(
            path const & bin_file, 
            CreateParamter const & paramter, 
            boost::system::error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return ec;
        }

        error_code Process::open(
            path const & bin_file, 
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return ec;
        }

        error_code Process::open(
            int pid, 
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return ec;
        }

        bool Process::is_alive(
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return false;
        }

        error_code Process::join(
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return ec;
        }

        error_code Process::timed_join(
            unsigned long milliseconds, 
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return ec;
        }

        error_code Process::detach(
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return ec;
        }

        error_code Process::kill(
            error_code & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return ec;
        }

        bool notify_wait(
            error_code const & ec)
        {
            SetLastError(ERROR_NOT_SUPPORTED);
            ec = last_system_error();
            return false;
        }

#endif

    } // namespace process
} // namespace framework
