// FileStream.cpp

#include "framework/Framework.h"
#include "framework/logger/FileStream.h"
#include "framework/filesystem/Path.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#ifndef BOOST_WINDOWS_API
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#endif

namespace framework
{
    namespace logger
    {

        FileStream::FileStream()
#ifdef BOOST_WINDOWS_API
            : handle_(INVALID_HANDLE_VALUE)
#else
            : fd_(-1)
#endif
            , app_(false)
            , daily_(false)
            , roll_(false)
            , size_(0)
        {
        }

        FileStream::~FileStream()
        {
            close();
        }

        void FileStream::load_config(
            ConfigModule & cm)
        {
            cm << CONFIG_PARAM_NAME_RDWR("file", file_);
            cm << CONFIG_PARAM_NAME_RDWR("append", app_);
            cm << CONFIG_PARAM_NAME_RDWR("daily", daily_);
            cm << CONFIG_PARAM_NAME_RDWR("size", size_);
            cm << CONFIG_PARAM_NAME_RDWR("roll", roll_);

            if (file_[0] == '$' 
                && file_[1] == 'L' 
                && file_[2] == 'O' 
                && file_[3] == 'G') {
                    file_ = 
                        framework::filesystem::log_path().file_string() 
                        + file_.substr(4);
            }

            backup_file();
        }

        void FileStream::write( 
            buffer_t const * bufs, 
            size_t len )
        {
            if (check_file()) {
                boost::mutex::scoped_lock lk(mutex_);
                if (check_file()) {
                    time_.update();
                    backup_file();
                }
            }

#ifdef BOOST_WINDOWS_API
            DWORD dw = 0;
            for ( size_t iLoop = 0; iLoop < len; ++iLoop ) {
                ::WriteFile(
                    handle_, 
                    bufs[iLoop].buf, 
                    bufs[iLoop].len, 
                    &dw, 
                    NULL);
            }

#else
            ::writev(
                fd_, 
                (iovec *)bufs, 
                len);
#endif
        }

        bool FileStream::open()
        {
#ifdef BOOST_WINDOWS_API
            handle_ = ::CreateFile(
                file_.c_str(), 
                GENERIC_READ | GENERIC_WRITE, 
                FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL, 
                OPEN_ALWAYS, 
                FILE_ATTRIBUTE_NORMAL, 
                NULL);
#else
            fd_ = ::open(
                file_.c_str(),
                O_CREAT | O_RDWR | O_EXCL, 
                00666);
#endif
            return is_open();
        }

        bool FileStream::is_open() const
        {
#ifdef BOOST_WINDOWS_API
            return handle_ == INVALID_HANDLE_VALUE;
#else
            return fd_ == -1;
#endif
        }

        bool FileStream::close()
        {
#ifdef BOOST_WINDOWS_API
            ::CloseHandle(
                handle_);
#else
            ::close(
                fd_);
#endif
            return true;
        }

        bool FileStream::reopen()
        {
#ifdef BOOST_WINDOWS_API
            return false;
#else
            int fd = ::open(
                file_.c_str(),
                O_CREAT | O_RDWR | O_EXCL, 
                00666);
            int fd2 = fd_;
            fd_ = fd;
            ::close(
                fd2);
            return is_open();
#endif
        }

        bool FileStream::seek(
            bool beg_or_end)
        {
#ifdef BOOST_WINDOWS_API
            DWORD dw = ::SetFilePointer(
                handle_, 
                0, 
                NULL, 
                beg_or_end ? FILE_BEGIN : FILE_END);
            return dw != INVALID_SET_FILE_POINTER;
#else
            off_t of = ::lseek(
                fd_, 
                beg_or_end ? SEEK_SET : SEEK_END, 
                0);
            return of != -1;
#endif
        }

        size_t FileStream::position()
        {
#ifdef BOOST_WINDOWS_API
            DWORD dw = ::SetFilePointer(
                handle_, 
                0, 
                NULL, 
                FILE_CURRENT);
            return dw; //INVALID_SET_FILE_POINTER
#else
            off_t of = ::lseek(
                fd_, 
                SEEK_CUR, 
                0);
            return (size_t)of;
#endif
        }

        bool FileStream::check_file()
        {
            if (size_ > 0 && position() >= size_)
                return true;
            if (daily_ && time_.check())
                return true;
            return false;
        }

        bool FileStream::backup_file()
        {
            if (roll_) {
                seek(true);
                return true;
            }
            bool app = app_;
            if (is_open()) {
#ifdef BOOST_WINDOWS_API
                close();
#endif
                app = false;
            }
            if (!app) {
                boost::filesystem::path ph(file_);
                boost::filesystem::path ph2(ph.parent_path() / "log_bak");
                boost::filesystem::path ph3(ph2 / (ph.leaf() + time_.time_str()));
                try {
                    boost::filesystem::create_directory(ph2);
                    if (boost::filesystem::exists(ph))
                        boost::filesystem::rename(ph, ph3);
                } catch (...) {
                }
            }
#ifdef BOOST_WINDOWS_API
            open();
#else
            reopen();
#endif
            if (app)
                seek(false);
            return true;
        }

    } // namespace logger
} // namespace framework
