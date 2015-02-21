// File.h

#ifndef _FRAMEWORK_FILE_SYSTEM_FILE_H_
#define _FRAMEWORK_FILE_SYSTEM_FILE_H_

#include "framework/container/Array.h"

#include <boost/filesystem/path.hpp>
#include <boost/asio/buffer.hpp>

namespace framework
{
    namespace filesystem
    {

        /* �򵥵ĳ����ļ���д��
         * std::fstream �ľ�����
         *   ��֧��һ������ɶ�λ���Ķ�д
         * boost::asio::stream_discriptor 
         *   ��֧�ֳ����ļ�
         * boost::asio::stream_handle
         *   ��Ҫ�ⲿ�򿪾��
         *   ��֧��seek
         * boost::asio::random_access_handle
         *   ��Ҫ�ⲿ�򿪾��
         *   ��Ҫ�ⲿ��¼��дλ��
         */

        class File
        {
        public:
            enum {
                f_exclude = 1, 
                f_trunc = 2, 
                f_create = 4, 
                f_read = 8, 
                f_write = 16, 
                f_read_write = f_read | f_write,

                fm_shared = 0x100 | 1, 
                fm_read = 0x100 | 2,
                fm_write = 0x100 | 4,
            };

        public:
            File();

            ~File();

        public:
            bool open(
                std::string const & name, 
                boost::system::error_code & ec);

            bool open(
                boost::filesystem::path const & path, 
                boost::system::error_code & ec);

            bool open(
                std::string const & name, 
                int flags, 
                boost::system::error_code & ec);

            bool open(
                boost::filesystem::path const & path, 
                int flags, 
                boost::system::error_code & ec);

            bool assign(
#ifdef BOOST_WINDOWS_API
                HANDLE handle, 
#else
                int fd, 
#endif
                boost::system::error_code & ec);

            bool is_open() const;

#ifdef BOOST_WINDOWS_API
            HANDLE native() const { return handle_; }
#else
            int native() const { return fd_; }
#endif

            void swap(
                File & r);

            bool close(
                boost::system::error_code & ec);

         public:
            void * map(
                boost::uint64_t offset, 
                size_t size, 
                int flags, 
                boost::system::error_code & ec);

            bool unmap(
                void * addr, 
                boost::uint64_t offset, 
                size_t size, 
                boost::system::error_code & ec);

        public:
            enum SeekDir
            {
#ifdef BOOST_WINDOWS_API
                beg = FILE_BEGIN, 
                cur = FILE_CURRENT, 
                end = FILE_END, 
#else
                beg = SEEK_SET, 
                cur = SEEK_CUR, 
                end = SEEK_END, 
#endif
            };

            bool seek(
                SeekDir dir, 
                boost::uint64_t offset, 
                boost::system::error_code & ec);

            boost::uint64_t tell(
                boost::system::error_code & ec);

        public:
            typedef boost::asio::mutable_buffer mutable_buffer_t;

            typedef framework::container::Array<mutable_buffer_t> mutable_buffers_t;

            typedef boost::asio::const_buffer const_buffer_t;

            typedef framework::container::Array<const_buffer_t> const_buffers_t;

            size_t read_some(
                mutable_buffer_t const & buffer, 
                boost::system::error_code & ec);

            size_t read_some(
                mutable_buffers_t const & buffers, 
                boost::system::error_code & ec);

            size_t write_some(
                const_buffer_t const & buffer, 
                boost::system::error_code & ec);

            size_t write_some(
                const_buffers_t const & buffers, 
                boost::system::error_code & ec);

        private:
#ifdef BOOST_WINDOWS_API
            HANDLE handle_;
#else
            int fd_;
#endif
        };

    } // namespace filesystem
} // namespace framework

#endif // _FRAMEWORK_FILE_SYSTEM_FILE_H_
