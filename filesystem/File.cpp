// Path.cpp

#include "framework/Framework.h"
#include "framework/filesystem/File.h"
#include "framework/system/ErrorCode.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  define MAX_PATH 256
#endif

namespace framework
{
    namespace filesystem
    {

        File::File()
#ifdef BOOST_WINDOWS_API
            : handle_(INVALID_HANDLE_VALUE)
#else
            : fd_(-1)
#endif
        {
        }

        File::~File()
        {
            boost::system::error_code ec;
            close(ec);
        }

        bool File::open(
            std::string const & name, 
            boost::system::error_code & ec)
        {
            return open(boost::filesystem::path(name), ec);
        }

        bool File::open(
            boost::filesystem::path const & path, 
            boost::system::error_code & ec)
        {
#ifdef BOOST_WINDOWS_API
            handle_ = ::CreateFileA(
                path.file_string().c_str(), 
                GENERIC_READ | GENERIC_WRITE, 
                FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL, 
                OPEN_ALWAYS, 
                FILE_ATTRIBUTE_NORMAL, 
                NULL);
#else
            fd_ = ::open(
                path.file_string().c_str(),
                O_CREAT | O_RDWR | O_EXCL, 
                00666);
#endif
            if (is_open()) {
                ec.clear();
                return true;
            } else {
                ec = framework::system::last_system_error();
                return false;
            }
        }

        bool File::assign(
#ifdef BOOST_WINDOWS_API
            HANDLE handle, 
#else
            int fd, 
#endif
            boost::system::error_code & ec)
        {
            if (is_open()) {
                close(ec);
            }
#ifdef BOOST_WINDOWS_API
            handle_ = handle; 
#else
            fd_ = fd;
#endif
            return true;
        }

        bool File::is_open() const
        {
#ifdef BOOST_WINDOWS_API
            return handle_ != INVALID_HANDLE_VALUE;
#else
            return fd_ != -1;
#endif
        }

        void File::swap(
            File & r)
        {
#ifdef BOOST_WINDOWS_API
            std::swap(handle_, r.handle_);
#else
            std::swap(fd_, r.fd_);
#endif
        }

        bool File::close(
            boost::system::error_code & ec)
        {
            ec.clear();
#ifdef BOOST_WINDOWS_API
            BOOL result = ::CloseHandle(
                handle_);
            if (result == FALSE) {
                ec = framework::system::last_system_error();
            } else {
                handle_ = INVALID_HANDLE_VALUE;
            }
#else
            int result = ::close(
                fd_);
            if (result == -1) {
                ec = framework::system::last_system_error();
            } else {
                fd_ = -1;
            }
#endif
            return !ec;
        }

        bool File::seek(
            SeekDir dir, 
            boost::uint64_t offset, 
            boost::system::error_code & ec)
        {
            ec.clear();
#ifdef BOOST_WINDOWS_API
            LARGE_INTEGER i;
            i.QuadPart = offset;
            BOOL result = ::SetFilePointerEx(
                handle_, 
                i, 
                NULL, 
                dir);
            if (result == FALSE) {
                ec = framework::system::last_system_error();
            }
#else
            off_t of = ::lseek(
                fd_, 
                dir, 
                offset);
            if (of == -1) {
                ec = framework::system::last_system_error();
            }
#endif
            return !ec;
        }

        boost::uint64_t File::tell(
            boost::system::error_code & ec)
        {
            ec.clear();
#ifdef BOOST_WINDOWS_API
            LARGE_INTEGER i;
            i.QuadPart = 0;
            BOOL result = ::SetFilePointerEx(
                handle_, 
                i, 
                &i, 
                FILE_CURRENT);
            if (result == FALSE) {
                ec = framework::system::last_system_error();
                i.QuadPart = boost::uint64_t(-1);
            }
            return i.QuadPart;
#else
            off_t of = ::lseek(
                fd_, 
                SEEK_CUR, 
                0);
            if (of == -1) {
                ec = framework::system::last_system_error();
            }
            return (size_t)of;
#endif
        }

        size_t File::read_some(
            mutable_buffer_t const & buffer, 
            boost::system::error_code & ec)
        {
#ifdef BOOST_WINDOWS_API
            DWORD dw = 0;
            BOOL result = ::ReadFile(
                handle_, 
                boost::asio::buffer_cast<LPVOID>(buffer), 
                boost::asio::buffer_size(buffer), 
                &dw, 
                NULL);
            if (result == FALSE) {
                ec = framework::system::last_system_error();
                if (ec.value() == ERROR_HANDLE_EOF) {
                    ec = boost::asio::error::eof;
                }
            }
            return dw;
#else
            int result = ::read(
                fd_, 
                boost::asio::buffer_cast<void *>(buffer), 
                boost::asio::buffer_size(buffer), 
                len);
            if (result == -1) {
                ec = framework::system::last_system_error();
                result = 0;
            } else if (result == 0) {
                ec = boost::asio::error::eof;
            }
            return result;
#endif
        }

        size_t File::read_some(
            mutable_buffers_t const & buffers, 
            boost::system::error_code & ec)
        {
#ifdef BOOST_WINDOWS_API
            size_t total = 0;
            ec.clear();
            for (mutable_buffers_t::const_iterator iter = buffers.begin(); 
                iter != buffers.end() && !ec; 
                ++iter) {
                    total += read_some(
                        *iter, 
                        ec);
                    if (ec.value() == boost::asio::error::eof && total > 0) {
                        ec.clear();
                        break;
                    }
            }
            return total;
#else
            if (buffers.size() > IOV_MAX) {
                ec = boost::system::error_code(EINVAL, boost::system::get_system_category());
                return 0;
            }
            struct iovec iov[IOV_MAX];
            for (mutable_buffers_t::const_iterator iter = buffers.begin(); 
                iter != buffers.end() && !ec; 
                ++iter) {
                    iov.iov_base = boost::asio::buffer_cast<void *>(*iter), 
                    iov.iov_len = boost::asio::buffer_size(*iter), 
            }
            int result = ::readv(
                fd_, 
                iov, 
                buffers.size());
            if (result == -1) {
                ec = framework::system::last_system_error();
                result = 0;
            } else if (result = 0) {
                ec = boost::asio::error::eof;
            }
            return result;
#endif
        }

        size_t File::write_some(
            const_buffer_t const & buffer, 
            boost::system::error_code & ec)
        {
#ifdef BOOST_WINDOWS_API
            DWORD dw = 0;
            BOOL result = ::WriteFile(
                handle_, 
                boost::asio::buffer_cast<LPCVOID>(buffer), 
                boost::asio::buffer_size(buffer), 
                &dw, 
                NULL);
            if (result == FALSE) {
                ec = framework::system::last_system_error();
            }
            return dw;
#else
            int result = ::write(
                fd_, 
                boost::asio::buffer_cast<const void *>(buffer), 
                boost::asio::buffer_size(buffer), 
                len);
            if (result == -1) {
                ec = framework::system::last_system_error();
                result = 0;
            }
            return result;
#endif
        }

        size_t File::write_some(
            const_buffers_t const & buffers, 
            boost::system::error_code & ec)
        {
#ifdef BOOST_WINDOWS_API
            size_t total = 0;
            ec.clear();
            for (const_buffers_t::const_iterator iter = buffers.begin(); 
                iter != buffers.end() && !ec; 
                ++iter) {
                    total += write_some(
                        *iter, 
                        ec);
            }
            return total;
#else
            if (buffers.size() > IOV_MAX) {
                ec = boost::system::error_code(EINVAL, boost::system::get_system_category());
                return 0;
            }
            struct iovec iov[IOV_MAX];
            for (const_buffers_t::const_iterator iter = buffers.begin(); 
                iter != buffers.end() && !ec; 
                ++iter) {
                    iov.iov_base = boost::asio::buffer_cast<LPCVOID>(*iter), 
                    iov.iov_len = boost::asio::buffer_size(*iter), 
            }
            int result = ::writev(
                fd_, 
                iov, 
                buffers.size());
            if (result == -1) {
                ec = framework::system::last_system_error();
                result = 0;
            }
            return result;
#endif
        }

    } // namespace filesystem
} // namespace framework
