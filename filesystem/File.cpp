// Path.cpp

#include "framework/Framework.h"
#include "framework/filesystem/File.h"
#include "framework/system/ErrorCode.h"
#include "framework/memory/MemoryPage.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <limits.h>
#  include <sys/mman.h>
#  include <sys/uio.h>
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
            return open(name, f_create | f_read_write, ec);
        }

        bool File::open(
            boost::filesystem::path const & path, 
            boost::system::error_code & ec)
        {
            return open(path, f_create | f_read_write, ec);
        }

        bool File::open(
            std::string const & name, 
            int flags, 
            boost::system::error_code & ec)
        {
            return open(boost::filesystem::path(name), flags, ec);
        }

        bool File::open(
            boost::filesystem::path const & path, 
            int flags, 
            boost::system::error_code & ec)
        {
#ifdef BOOST_WINDOWS_API
            DWORD dwDesiredAccess = 0;
            if (flags & f_read) {
                dwDesiredAccess |= GENERIC_READ;
            }
            if (flags & f_write) {
                dwDesiredAccess |= GENERIC_WRITE;
            }
            DWORD dwCreationDispositionTable[] = {
                OPEN_EXISTING, // 
                OPEN_EXISTING, // f_exclude
                TRUNCATE_EXISTING, // f_trunc
                TRUNCATE_EXISTING, // f_exclude | f_trunc
                OPEN_ALWAYS, // f_create
                CREATE_NEW, // f_create | f_exclude
                CREATE_ALWAYS, // f_create | f_trunc
                CREATE_NEW, // f_create | f_exclude | f_trunc
            };
            DWORD dwCreationDisposition = dwCreationDispositionTable[flags & 0x07];
            handle_ = ::CreateFileA(
                path.string().c_str(), 
                dwDesiredAccess, 
                FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL, 
                dwCreationDisposition, 
                FILE_ATTRIBUTE_NORMAL, 
                NULL);
#else
            int f = 0;
            if (flags & f_read) {
                if (flags & f_write) {
                    f |= O_RDWR;
                } else {
                    f |= O_RDONLY;
                }
            } else {
                f |= O_WRONLY;
            }
            if (flags & f_create) {
                f |= O_CREAT;
                if (flags & f_exclude) {
                    f |= O_EXCL;
                }
            }
            if (flags & f_trunc) {
                f |= O_TRUNC;
            }
            fd_ = ::open(
                path.string().c_str(),
                f, 
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

        void * File::map(
            boost::uint64_t offset, 
            size_t size, 
            int flags, 
            boost::system::error_code & ec)
        {
            boost::uint64_t o = offset;
            framework::memory::MemoryPage::align_page(o, size);
#ifdef BOOST_WINDOWS_API
            DWORD dwFlags = 0;
            if (flags & fm_read)
                dwFlags = PAGE_READONLY;
            if (flags & fm_write)
                dwFlags = PAGE_READWRITE;
            //if (flags & fm_shared)
            //    dwFlags |= 0;
            //else
            //    dwFlags |= PAGE_WRITECOPY;
            HANDLE hFileMap = ::CreateFileMappingA(
                handle_, 
                NULL, 
                dwFlags, 
                0, 
                0, 
                NULL);

            if (hFileMap == NULL) {
                ec = framework::system::last_system_error();
                return NULL;
            }

            DWORD dwDesiredAccess = 0;
            if (flags & fm_read)
                dwDesiredAccess = FILE_MAP_READ;
            if (flags & fm_write)
                dwDesiredAccess = FILE_MAP_WRITE;
            //if (flags & fm_shared)
            //    dwDesiredAccess |= 0;
            //else
            //    dwDesiredAccess |= FILE_MAP_COPY;
            void * addr = MapViewOfFile(
                hFileMap, 
                dwDesiredAccess, 
                (DWORD)(o >> 32), 
                (DWORD)o, 
                (SIZE_T)size);

            if (addr == NULL) {
                ec = framework::system::last_system_error();
                ::CloseHandle(
                    hFileMap);
                return NULL;
            }

            ::CloseHandle(
                hFileMap);
            ec.clear();
            
#else
            int p = 0;
            int f = 0;
            if (flags & fm_read)
                p |= PROT_READ;
            if (flags & fm_write)
                p |= PROT_WRITE;
            if (flags & fm_shared)
                f |= MAP_SHARED;
            else
                f |= MAP_PRIVATE;
            void * addr = ::mmap(
                NULL, 
                size, 
                p, 
                f, 
                fd_,
                o);

            if (addr == MAP_FAILED) {
                ec = framework::system::last_system_error();
                return NULL;
            }
            ec.clear();
#endif
            return (char *)addr + size_t(offset - o);
        }

        bool File::unmap(
            void * addr, 
            boost::uint64_t offset, 
            size_t size, 
            boost::system::error_code & ec)
        {
            boost::uint64_t o = offset;
            framework::memory::MemoryPage::align_page(o, size);
            addr = (char *)addr - size_t(offset - o);
#ifdef BOOST_WINDOWS_API
            BOOL b = ::UnmapViewOfFile(
                addr);
            if (b == FALSE) {
                ec = framework::system::last_system_error();
                return false;
            }
            ec.clear();
            return true;
#else
            int r = ::munmap(
                addr, 
                size);

            if (r == -1) {
                ec = framework::system::last_system_error();
                return false;
            }
            ec.clear();
            return true;
#endif
        }

        bool File::seek(
            SeekDir dir, 
            boost::uint64_t offset, 
            boost::system::error_code & ec)
        {
            ec.clear();
#ifdef BOOST_WINDOWS_API
            UINT dirs[] = {FILE_BEGIN, FILE_CURRENT, FILE_END};
            LARGE_INTEGER i;
            i.QuadPart = offset;
            BOOL result = ::SetFilePointerEx(
                handle_, 
                i, 
                NULL, 
                dirs[dir]);
            if (result == FALSE) {
                ec = framework::system::last_system_error();
            }
#else
            int dirs[] = {SEEK_SET, SEEK_CUR, SEEK_END};
            off_t of = ::lseek(
                fd_, 
                offset, 
                dirs[dir]);
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
                0,
                SEEK_CUR);
            if (of == -1) {
                ec = framework::system::last_system_error();
            }
            return (boost::uint64_t)of;
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
            } else if (dw == 0) {
                ec = boost::asio::error::eof;
            }
            return dw;
#else
            int result = ::read(
                fd_, 
                boost::asio::buffer_cast<void *>(buffer), 
                boost::asio::buffer_size(buffer));
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
            for (size_t i = 0; i < buffers.size() && !ec; ++i) {
                iov[i].iov_base = boost::asio::buffer_cast<void *>(buffers[i]);
                iov[i].iov_len = boost::asio::buffer_size(buffers[i]);
            }
            int result = ::readv(
                fd_, 
                iov, 
                buffers.size());
            if (result == -1) {
                ec = framework::system::last_system_error();
                result = 0;
            } else if (result == 0) {
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
                boost::asio::buffer_size(buffer));
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
            for (size_t i = 0; i < buffers.size() && !ec; ++i) {
                iov[i].iov_base = (void *)boost::asio::buffer_cast<void const *>(buffers[i]);
                iov[i].iov_len = boost::asio::buffer_size(buffers[i]);
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
