#ifndef _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYWINDOWS_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYWINDOWS_H_

#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"
using namespace framework::system;

#include <Windows.h>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#define INVALID_RET_VALUE NULL

namespace framework
{
    namespace memory
    {

        namespace detail
        {
            typedef HANDLE SHM_ID;

            std::string name_key(
                boost::uint32_t iid, 
                boost::uint32_t key)
            {
                return std::string("Global\\SharedMemory_") 
                    + framework::simple_version_string() + "_" 
                    + format(iid) 
                    + "_" + format(key);
            }

            SHM_ID Shm_create( 
                boost::uint32_t uni_id,
                boost::uint32_t key, 
                boost::uint32_t size,
                error_code & ec)
            {
                HANDLE id = ::CreateFileMapping(
                    INVALID_HANDLE_VALUE, 
                    NULL, 
                    PAGE_READWRITE, 
                    0, 
                    size, 
                    name_key(uni_id, key).c_str());
                if (!id || GetLastError() == ERROR_ALREADY_EXISTS) {
                    if (id)
                        CloseHandle(id);
                    ec = last_system_error();
                    return ( SHM_ID )NULL;
                }
                return ( SHM_ID )id;
            }

            void * Shm_map(
                boost::uint32_t uni_id,
                boost::uint32_t key,
                SHM_ID id,
                boost::uint32_t size,
                bool iscreat,
                error_code & ec )
            {
                void * p = MapViewOfFile(
                    id, 
                    FILE_MAP_ALL_ACCESS, 
                    0, 
                    0, 
                    0);
                if (p == NULL) {
                    ec = last_system_error();
                    if ( iscreat ) 
                    {           
                        ::CloseHandle(id);
                    }

                    return NULL;
                }

                return p;
            }

            SHM_ID Shm_open( 
                boost::uint32_t uni_id,
                boost::uint32_t key,
                error_code & ec)
            {
                HANDLE id = ::OpenFileMapping(
                    FILE_MAP_ALL_ACCESS, 
                    FALSE, 
                    name_key(uni_id, key).c_str());
                if (!id) {
                    ec = last_system_error();
                    return ( SHM_ID )NULL;
                }

                return ( SHM_ID )id;
            }

            void Shm_unmap( void * addr, size_t size )
            {
                UnmapViewOfFile( addr );
            }

            void Shm_close( SHM_ID id )
            {
                CloseHandle( id );
            }

            bool Shm_destory( 
                int uni_id, 
                int key, 
                SHM_ID id)
            {  
                return false;
            }
        } // namespace detail

    } // namespace memory
} // namespace framework
#endif // _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYWINDOWS_H_