// SharedMemoryWindows.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WINDOWS_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WINDOWS_H_

#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"

#define SHM_NULL NULL

namespace framework
{
    namespace memory
    {

        namespace detail
        {
            typedef HANDLE shm_t;

            std::string name_key(
                boost::uint32_t iid, 
                boost::uint32_t key)
            {
                return std::string("Global\\SharedMemory_") 
                    + framework::simple_version_string() + "_" 
                    + format(iid) 
                    + "_" + format(key);
            }

            shm_t Shm_create( 
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
                    ec = last_system_error();
                    if (id)
                        CloseHandle(id);
                    return ( shm_t )NULL;
                }
                return ( shm_t )id;
            }

            void * Shm_map(
                shm_t id,
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
                    return NULL;
                }

                return p;
            }

            shm_t Shm_open( 
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
                    return ( shm_t )NULL;
                }

                return ( shm_t )id;
            }

            void Shm_unmap( void * addr, size_t size )
            {
                UnmapViewOfFile( addr );
            }

            void Shm_close( shm_t id )
            {
                CloseHandle( id );
            }

            bool Shm_destory( 
                int uni_id, 
                int key,
                boost::system::error_code & ec)
            {
                ec.clear();
                return false;
            }

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WINDOWS_H_
