// Process.cpp

#include <sys/types.h> // for pid_t
#include <stdlib.h> // for setenv etc.
#include <sys/wait.h> // for waitpid
#include <unistd.h> // for fork
#include <fcntl.h> // for open
#include <signal.h> // for sigaction, kill
#include <poll.h> // for poll

namespace framework
{
    namespace process
    {

        namespace detail
        {
            struct process_data_base
            {
                bool is_alive;
                pid_t pid;
                pid_t ppid;
                std::vector<ProcessInfo> * handle;
                int status;

                process_data_base()
                    : is_alive(true)
                    , pid(0)
                    , handle(NULL)
                    , status(0)
                {
                }
            };
        }

        error_code Process::create(
            path const & bin_file, 
            CreateParamter const & paramter, 
            boost::system::error_code & ec)
        {
            if (is_open())
                return ec = already_open;

            std::string bin_file_path = bin_file.file_string();
            const char * filename = bin_file_path.c_str();
            std::vector<char *> cmdArr(paramter.args.size() + 2, 0);
            cmdArr[0] = &bin_file_path[0];
            for(size_t i = 0; i < paramter.args.size(); ++i) {
                cmdArr[i + 1] = (char *)paramter.args[i].c_str();    
            }
            int pipefd[2];
            if (paramter.wait) {
                ::pipe(pipefd);
            }
            pid_t pid = fork();
            if (pid < 0) {
                if (paramter.wait) {
                    ::close(pipefd[0]);
                    ::close(pipefd[1]);
                }
                ec = last_system_error();
            } else if (pid == 0) {
                int nfd = ::open("/dev/null", O_RDWR);
                if (paramter.infd == -2)
                    dup2(nfd, STDIN_FILENO);
                else if (paramter.infd != -1)
                    dup2(paramter.infd, STDIN_FILENO);
                if (paramter.outfd == -2)
                    dup2(nfd, STDOUT_FILENO);
                else if (paramter.outfd != -1)
                    dup2(paramter.outfd, STDOUT_FILENO);
                if (paramter.errfd == -2)
                    dup2(nfd, STDERR_FILENO);
                else if (paramter.errfd != -1)
                    dup2(paramter.errfd, STDERR_FILENO);
                ::close(nfd);
                int i = 3;
                if (paramter.wait) {
                    dup2(pipefd[1], 3);
                    i = 4;
                    ::setenv("FRAMEWORK_PROCESS_WAIT_FILE", "3", 1 /* overwrite */);
                }
#define MAXFILE 65535
                for(; i < MAXFILE; ++i)    //关闭父进程打开的文件描述符，主要是为了关闭socket
                {
#ifdef __ANDROID__
                    if ( i < 8 || i > 18 )
#endif
                        ::close(i);
                }
#undef MAXFILE
                if (execvp(filename, &cmdArr.at(0)) < 0) {
                    notify_wait(last_system_error());
                    _exit(errno);
                }
                return error_code();
            } else {
                ec = error_code();
                ProcessInfo pi;
                pi.pid = pid;
                pi.bin_file = bin_file;
                data_ = new detail::process_data_base;
                data_->pid = pid;
                data_->ppid = ::getpid();
                data_->handle = new std::vector<ProcessInfo>(1, pi);
                if (paramter.wait) {
                    ::close(pipefd[1]);
                    char buffer[64];
                    int len = ::read(pipefd[0], buffer, sizeof(buffer));
                    char * p = ::strchr(buffer, ':');
                    if (p) {
                        *p++ = '\0';
                        ec.assign((int)framework::process::detail::parse(p, buffer + len - p), 
                            boost::system::error_category::find_category(buffer));
                    } else {
                        ec = not_alive;
                    }
                    ::close(pipefd[0]);
                }
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
                    data_ = new detail::process_data_base;
                    data_->pid = pis[0].pid;
                    data_->ppid = 0;
                    data_->handle = new std::vector<ProcessInfo>;
                    for (size_t i = 0; i < pis.size(); ++i) {
                        data_->handle->push_back(pis[i]);
                    }
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
            ProcessInfo info;
            if (get_process_info(info, pid, "", ec)) {
                data_->handle = new std::vector<ProcessInfo>(1, info);
            } else {
                LOG_DEBUG("open (no such process): " << ec.message());
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
                if (data_->ppid == ::getpid()) {
                    int ret = ::waitpid(data_->pid, &data_->status, WNOHANG);
                    if (ret == data_->pid) {
                        data_->is_alive = false;
                    } else if (ret == 0) {
                        data_->is_alive = true;
                    } else {
                        ec = last_system_error();
                        data_->is_alive = false;
                    }
                } else {
                    ProcessInfo info;
                    data_->is_alive = false;
                    while (!data_->handle->empty()) {
                        ProcessInfo & pi = data_->handle->front();
                        if (!get_process_info(info, pi.pid, "", ec)) {
                            LOG_TRACE("is_alive (no such process) " << pi.pid);
                            pi = data_->handle->back();
                            data_->handle->pop_back();
                            continue;
                        }
                        if (!has_process_name(pi.bin_file, info.bin_file)) {
                            LOG_TRACE("is_alive (not match process) " << pi.pid 
                                << " " << info.bin_file);
                            pi = data_->handle->back();
                            data_->handle->pop_back();
                            continue;
                        }
                        ProcessStat stat;
                        if ((ec = get_process_stat(pi.pid, stat))) {
                            LOG_TRACE("is_alive (not process) " << pi.pid);
                            pi = data_->handle->back();
                            data_->handle->pop_back();
                            continue;
                        }
                        if (stat.state != ProcessStat::zombie && stat.state != ProcessStat::dead) {
                            data_->is_alive = true;
                        } else {
                            LOG_TRACE("is_alive (process dead) " << pi.pid);
                        }
                        break;
                    }
                    if (!data_->is_alive) {
                        LOG_TRACE("is_alive (process not exist) " << data_->pid);
                    }
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
                if (::waitpid(data_->pid, &data_->status, 0) > 0) {
                    data_->is_alive = false;
                } else {
                    ec = last_system_error();
                }
            }
            return ec;
        }

        static int write_fd = -1;
        static void sig_child(int signo)
        {
            char c = 0;
            ::write(write_fd, &c, 1);
        }

        error_code Process::timed_join(
            unsigned long milliseconds, 
            error_code & ec)
        {
            if (is_alive(ec)) {
                if (data_->ppid == ::getpid()) {
                    if (::waitpid(data_->pid, &data_->status, WNOHANG) > 0) {
                        data_->is_alive = false;
                    } else {
                        int pipefd[2];
                        ::pipe(pipefd);
                        write_fd = pipefd[1];
                        struct sigaction sa;
                        struct sigaction sa_old;
                        sa.sa_handler = sig_child;
                        sa.sa_flags = SA_NODEFER;
                        sigaction(SIGCHLD, &sa, &sa_old);
                        struct pollfd fd;
                        fd.fd = pipefd[0];
                        fd.events = POLLIN;
                        while (true) {
                            int ret = ::waitpid(data_->pid, &data_->status, WNOHANG);
                            if (ret < 0) {
                                ec = last_system_error();
                                break;
                            } else if (ret > 0) {
                                ec.clear();
                                break;
                            }
                            ret = ::poll(&fd, 1, milliseconds);
                            if (ret < 0) {
                                if (errno != EINTR) {
                                    ec = last_system_error();
                                    break;
                                }
                            } else if (ret == 0) {
                                ec = error_code(ETIMEDOUT, boost::system::get_system_category());
                                break;
                            }
                        }
                        sigaction(SIGALRM, &sa_old, NULL);
                        ::close(pipefd[0]);
                        ::close(pipefd[1]);
                    }
                } else {
                    while (is_alive(ec) && milliseconds) {
                        usleep(100 * 1000);
                        milliseconds = milliseconds > 100 ? milliseconds - 100 : 0;
                    }
                    if (!ec && milliseconds == 0) {
                        ec = error_code(ETIMEDOUT, boost::system::get_system_category());
                    }
                }
            }
            return ec;
        }

        error_code Process::signal(
            Signal sig, 
            error_code & ec)
        {
            if (is_alive(ec)) {
                ::kill(data_->pid, sig.value());
                ec = last_system_error();
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
            if (data_->handle) {
                delete data_->handle;
            }
            delete data_;
            data_ = NULL;
            return ec = error_code();
        }

        error_code Process::kill(
            error_code & ec)
        {
            if (is_alive(ec)) {
                if (::kill(data_->pid, SIGKILL) == 0) {
                    if (data_->ppid == ::getpid()) {
                        ::waitpid(data_->pid, &data_->status, 0);
                        ec = last_system_error();
                    }
                    if (data_->handle) {
                        delete data_->handle;
                    }
                } else {
                    ec = last_system_error();
                }
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
            if (WIFEXITED(data_->status)) {
                return WEXITSTATUS(data_->status);
            } else {
                return 0;
            }
        }

        error_code Process::stat(
            ProcessStat & stat) const
        {
            error_code ec;
            if (data_ && data_->is_alive) {
                ec = get_process_stat(data_->pid, stat);
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
                ec = get_process_statm(data_->pid, statm);
            } else {
                ec = not_alive;
            }
            return ec;
        }

        bool notify_wait(
            error_code const & ec)
        {
            char * value;
            if ((value = ::getenv("FRAMEWORK_PROCESS_WAIT_FILE"))) {
                int fd = detail::parse(value, strlen(value));
                ::unsetenv("FRAMEWORK_PROCESS_WAIT_FILE");
                char buffer[64];
                size_t len = strlen(ec.category().name());
                memcpy(buffer, ec.category().name(), len);
                buffer[len++] = ':';
                len += detail::format(buffer + len, ec.value());
                ::write(fd, buffer, len);
                ::close(fd);
                return true;
            }
            return false;
        }

    } // namespace process
} // namespace framework
