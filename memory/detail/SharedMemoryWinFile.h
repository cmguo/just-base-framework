// SharedMemoryWinFile.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WIN_FILE_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WIN_FILE_H_

#include "framework/string/Format.h"
#include "framework/filesystem/Path.h"
#include "framework/system/ErrorCode.h"

#define SHM_NULL INVALID_HANDLE_VALUE

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
                std::string file_name = framework::filesystem::framework_temp_path().string() + "/SharedMemory_";
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
                error_code & ec)
            {
                HANDLE hFile = ::CreateFile(
                    name_key(uni_id, key).c_str(), 
                    GENERIC_READ | GENERIC_WRITE, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE, 
                    NULL, 
                    CREATE_NEW, 
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL);
                if (hFile == INVALID_HANDLE_VALUE) {
                    ec = last_system_error();
                    return SHM_NULL;
                }
                DWORD dw = ::SetFilePointer(
                    hFile, 
                    size, 
                    NULL, 
                    FILE_BEGIN);
                if (dw == INVALID_SET_FILE_POINTER) {
                    ec = last_system_error();
                    ::CloseHandle(hFile);
                    ::DeleteFile(
                        name_key(uni_id, key).c_str());
                    return SHM_NULL;
                }
                BOOL b = ::SetEndOfFile(
                    hFile);
                if (b == FALSE) {
                    ec = last_system_error();
                    ::CloseHandle(hFile);
                    ::DeleteFile(
                        name_key(uni_id, key).c_str());
                    return SHM_NULL;
                }
                return hFile;
            }

            shm_t Shm_open( 
                boost::uint32_t uni_id,
                boost::uint32_t key,
                error_code & ec)
            {
                HANDLE hFile = ::CreateFile(
                    name_key(uni_id, key).c_str(), 
                    GENERIC_READ | GENERIC_WRITE, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE, 
                    NULL, 
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL);
                if (hFile == INVALID_HANDLE_VALUE) {
                    ec = last_system_error();
                    return SHM_NULL;
                }
                return hFile;
            }

            void * Shm_map(
                shm_t id, 
                boost::system::error_code & ec )
            {
                HANDLE hFileMap = ::CreateFileMapping(
                    id, 
                    NULL, 
                    PAGE_READWRITE, 
                    0, 
                    0, 
                    NULL);
                if (hFileMap == NULL) {
                    ec = last_system_error();
                    return SHM_NULL;
                }
                void * p = MapViewOfFile(
                    id, 
                    FILE_MAP_ALL_ACCESS, 
                    0, 
                    0, 
                    0);
                if (p == NULL) {
                    ec = last_system_error();
                    ::CloseHandle(
                        hFileMap);
                    return NULL;
                }
                // Mapped views of a file mapping object maintain internal references to the object, 
                // and a file mapping object does not close until all references to it are released. 
                // Therefore, to fully close a file mapping object, an application must unmap all 
                // mapped views of the file mapping object by calling UnmapViewOfFile and close the 
                // file mapping object handle by calling CloseHandle. These functions can be called 
                // in any order.
                ::CloseHandle(
                    hFileMap);
                return p;
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
                BOOL b = ::DeleteFile(
                    name_key(uni_id, key).c_str());
                ec = last_system_error();
                return b;
            }

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WIN_FILE_H_
