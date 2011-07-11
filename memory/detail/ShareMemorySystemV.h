#ifndef _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYSYSTEMV_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYSYSTEMV_H_

#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"
#include "framework/filesystem/Path.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INVALID_RET_VALUE -1

namespace framework
{
    namespace memory
    {

        namespace detail
        {
            typedef int SHM_ID;

            std::string tmp_file_name(
                boost::uint32_t iid)
            {
                std::string file_name = framework::filesystem::framework_temp_path().string() + "/SharedMemory_";
                file_name += framework::simple_version_string();
                file_name += "_";
                file_name += format(iid);
                int fd = ::open(file_name.c_str(), O_CREAT, S_IRWXG | S_IRWXO | S_IRWXU);
                ::close(fd);
                return file_name;
            }

            key_t name_key(
                boost::uint32_t iid, 
                boost::uint32_t key)
            {
                static std::string file_name = tmp_file_name(iid);
                return ftok(file_name.c_str(), key);
            }

            SHM_ID Shm_create( 
                boost::uint32_t uni_id,
                boost::uint32_t key, 
                boost::uint32_t size, 
                 boost::system::error_code & ec)
            {
                int id = ::shmget(
                    name_key( uni_id, key ), 
                    size, 
                    IPC_CREAT | IPC_EXCL | 0666);
                if (id == -1) {
                    ec = framework::system::last_system_error();
                    return ( SHM_ID )-1;
                }

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
                void * p;
                if ( iscreat )
                {
                    p = ::shmat(
                        id, 
                        NULL, 
                        0);
                    if (p == NULL) {
                        ec = framework::system::last_system_error();
                        ::shmctl(
                            id, 
                            IPC_RMID, 
                            NULL);
                        return NULL;
                    }
                }
                else
                {
                    p = ::shmat(
                        id, 
                        0, 
                        0);
                    if (p == (void *)-1) {
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
                int id = ::shmget(
                    name_key(uni_id, key), 0, 0666);
                if (id == -1) {
                    ec = framework::system::last_system_error();
                    return ( SHM_ID )-1;
                }

                return ( SHM_ID )id;
            }

            void Shm_unmap( void * addr, size_t size )
            {
                shmdt(addr);
            }

            void Shm_close( SHM_ID id )
            {
            }

            bool Shm_destory( 
                int uni_id, 
                int key, 
                SHM_ID id)
            {  
                bool ret = true;
                shmid_ds sds;
                if (shmctl(id, IPC_STAT, &sds) != -1 && sds.shm_nattch == 0)
                    shmctl(id, IPC_RMID, NULL);
                else
                    ret = false;
                return ret;
            }
        } // namespace detail

    } // namespace memory
} // namespace framework
#endif // _FRAMEWORK_MEMORY_DETAIL_SHAREMEMORYSYSTEMV_H_
