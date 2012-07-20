// SharedMemoryPosix.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_POSIX_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_POSIX_H_

#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SHM_NULL -1

namespace framework
{
    namespace memory
    {

        namespace detail
        {
            typedef int shm_t;

            std::string name_key(
                boost::uint32_t iid, 
                boost::uint32_t key)
            {
                std::string file_name = "SharedMemory_";
                file_name += framework::simple_version_string();
                file_name += "_";
                file_name += format(iid);
                file_name += "_" + format( key );

                return file_name;
            }

            shm_t Shm_create( 
                boost::uint32_t uni_id,
                boost::uint32_t key, 
                boost::uint32_t size, 
                boost::system::error_code & ec)
            {
                int id = ::shm_open(
                    name_key( uni_id, key ).c_str(),
                    O_CREAT | O_RDWR | O_EXCL, 
                    00666);

                if (id == -1 ){
                    ec = framework::system::last_system_error();
                    return ( shm_t )-1;
                }

                ::ftruncate(id, size);

                framework::process::read_lock( id, 0, SEEK_SET, 0 );
                return ( shm_t )id;
            }

            shm_t Shm_open( 
                boost::uint32_t uni_id,
                boost::uint32_t key,
                boost::system::error_code & ec)
            {
                int id = ::shm_open(
                    name_key( uni_id, key ).c_str(),
                    O_RDWR,
                    0666);

                if (id == -1 ){
                    ec = framework::system::last_system_error();
                    return ( shm_t )-1;
                }

                framework::process::read_lock( id, 0, SEEK_SET, 0 );
                return ( shm_t )id;
            }

            void * Shm_map(
                shm_t id,
                boost::system::error_code & ec )
            {
                void * p = NULL;
                struct stat stat_;
                ::fstat( id, &stat_ );
                p = ::mmap(
                    NULL,
                    stat_.st_size,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    id,
                    0);

                if ( p == MAP_FAILED ){
                    ec = framework::system::last_system_error();
                    return NULL;
                }
                return p;
            }

            void Shm_unmap( void * addr, size_t size )
            {
                ::munmap( addr, size );
            }

            void Shm_close( shm_t id )
            {
                ::close(id);
            }

            bool Shm_destory( 
                int uni_id, 
                int key,
                boost::system::error_code & ec)
            {  
                int id = ::shm_open(
                    name_key( uni_id, key ).c_str(),
                    O_RDWR,
                    0666);
                if (id == SHM_NULL) {
                    ec = framework::system::last_system_error();
                    return false;
                }
                int ret = true;
                if ( framework::process::write_lock( id, 0, SEEK_SET, 0 ) != -1 )
                {
                    ::close(id);
                    if ( -1 == ::shm_unlink( name_key( uni_id, key ).c_str() ) ) {
                        ec = framework::system::last_system_error();
                        ret = false;
                    }
                }
                else
                {
                    ec = framework::system::last_system_error();
                    ::close(id);
                    ret = false;
                }
                return ret;
            }
        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_POSIX_H_
