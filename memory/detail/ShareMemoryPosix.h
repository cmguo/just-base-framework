#ifndef _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYPOSIX_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYPOSIX_H_

#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"
#include "framework/process/FileMutex.h"

//#include <boost/filesystem/operations.hpp>
//#include <boost/interprocess/shared_memory_object.hpp>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INVALID_RET_VALUE -1

namespace framework
{
    namespace memory
    {

        namespace detail
        {
            typedef int SHM_ID;

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

            SHM_ID Shm_create( 
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
                    return ( SHM_ID )-1;
                }

                ::ftruncate(id, size);

                framework::process::read_lock( id, 0, SEEK_SET, 0 );
                return ( SHM_ID )id;
            }

            void * Shm_map(
                boost::uint32_t uni_id,
                boost::uint32_t key,
                SHM_ID id,
                boost::uint32_t size,
                bool iscreat,
                 boost::system::error_code & ec )
            {
                void * p = NULL;
                if ( iscreat )
                {
                    p = ::mmap(
                        NULL,
                        size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        id,
                        0);

                    if ( p == MAP_FAILED ){
                        ec = framework::system::last_system_error();
                        ::shm_unlink( name_key( uni_id, key ).c_str() );
                        return NULL;
                    }
                }
                else
                {
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
                }

                return p;
            }

            SHM_ID Shm_open( 
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
                    return ( SHM_ID )-1;
                }

                framework::process::read_lock( id, 0, SEEK_SET, 0 );
                return ( SHM_ID )id;
            }

            void Shm_unmap( void * addr, size_t size )
            {
                ::munmap( addr, size );
            }

            void Shm_close( SHM_ID id )
            {
            }

            bool Shm_destory( 
                int uni_id, 
                int key, 
                SHM_ID id)
            {  
                int ret = true;
                if ( framework::process::write_lock( id, 0, SEEK_SET, 0 ) != -1 )
                {
                    if ( -1 == ::shm_unlink( name_key( uni_id, key ).c_str() ) ) 
                        ret = false;
                }
                else
                {
                    ret = false;
                }
                return ret;
            }
        } // namespace detail

    } // namespace memory
} // namespace framework
#endif // _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYPOSIX_H_
