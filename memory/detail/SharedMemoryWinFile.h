// SharedMemoryWinFile.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WIN_FILE_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WIN_FILE_H_

#include "framework/memory/detail/SharedMemoryImpl.h"

namespace framework
{
    namespace memory
    {

        namespace detail
        {

            class SharedMemoryWinFile
                : public SharedMemoryImpl
            {
            private:
                bool create( 
                    map_id_t* id, 
                    boost::uint32_t iid,
                    boost::uint32_t key, 
                    boost::uint32_t size,
                    boost::system::error_code & ec)
                {
                    std::string name = key_path(iid, key);

                    ObjectWrapper ow_destroy;
                    ObjectWrapper ow;
                    ErrorCodeWrapper ecw(ec);

                    HANDLE hFile = ::CreateFileA(
                        name.c_str(), 
                        GENERIC_READ | GENERIC_WRITE, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE, 
                        NULL, 
                        CREATE_NEW, 
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL);

                    if (hFile == INVALID_HANDLE_VALUE) {
                        return false;
                    }

                    ow_destroy.reset(name.c_str(), ::DeleteFileA);
                    ow.reset(hFile, ::CloseHandle);

                    DWORD dw = ::SetFilePointer(
                        hFile, 
                        size, 
                        NULL, 
                        FILE_BEGIN);

                    if (dw == INVALID_SET_FILE_POINTER) {
                        return false;
                    }

                    BOOL b = ::SetEndOfFile(
                        hFile);

                    if (b == FALSE) {
                        return NULL;
                    }

                    *id = ow.release();
                    ow_destroy.release();

                    return true;
                }

                bool open( 
                    map_id_t* id, 
                    boost::uint32_t iid,
                    boost::uint32_t key,
                    error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    HANDLE hFile = ::CreateFileA(
                        key_path(iid, key).c_str(), 
                        GENERIC_READ | GENERIC_WRITE, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE, 
                        NULL, 
                        OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL);

                    if (hFile == INVALID_HANDLE_VALUE) {
                        return false;
                    }

                    *id = (map_id_t)hFile;

                    return true;
                }

                void * map(
                    map_id_t id, 
                    boost::uint32_t size,
                    boost::system::error_code & ec )
                {
                    ObjectWrapper ow;
                    ErrorCodeWrapper ecw(ec);

                    HANDLE hFile = ObjectWrapper::cast_object<HANDLE>(id);

                    HANDLE hFileMap = ::CreateFileMappingA(
                        hFile, 
                        NULL, 
                        PAGE_READWRITE, 
                        0, 
                        0, 
                        NULL);

                    if (hFileMap == NULL) {
                        return NULL;
                    }

                    ow.reset(hFileMap, ::CloseHandle);

                    void * p = ::MapViewOfFile(
                        hFileMap, 
                        FILE_MAP_ALL_ACCESS, 
                        0, 
                        0, 
                        0);

                    if (p == NULL) {
                        return NULL;
                    }
                    // Mapped views of a file mapping object maintain internal references to the object, 
                    // and a file mapping object does not close until all references to it are released. 
                    // Therefore, to fully close a file mapping object, an application must unmap all 
                    // mapped views of the file mapping object by calling UnmapViewOfFile and close the 
                    // file mapping object handle by calling CloseHandle. These functions can be called 
                    // in any order.

                    // ow.release();

                    return p;
                }

                bool unmap(
                    void * addr, 
                    boost::uint32_t size,
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    BOOL b = ::UnmapViewOfFile(
                        addr);

                    if (b == FALSE) {
                        return false;
                    }

                    return true;
                }

                bool close(
                    map_id_t id, 
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    HANDLE hFile = ObjectWrapper::cast_object<HANDLE>(id);

                    BOOL b = :: CloseHandle(
                        hFile);

                    if (b == FALSE) {
                        return false;
                    }

                    return true;
                }

                bool destory( 
                    boost::uint32_t iid, 
                    boost::uint32_t key, 
                    boost::system::error_code & ec)
                {
                    ErrorCodeWrapper ecw(ec);

                    BOOL b = ::DeleteFileA(
                        key_path(iid, key).c_str());

                    if (b == FALSE) {
                        return false;
                    }

                    return true;
                }
            };

            static SharedMemoryWinFile shared_memory_win_file;

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_WIN_FILE_H_
