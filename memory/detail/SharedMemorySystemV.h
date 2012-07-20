// SharedMemorySystemV.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_SYSTEM_V_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_SYSTEM_V_H_

#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"
#include "framework/filesystem/Path.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_NULL -1

namespace framework
{
    namespace memory
    {

        namespace detail
        {
            typedef int shm_t;

            std::string tmp_file_name(
                boost::uint32_t iid)
            {
                std::string file_name = framework::filesystem::framework_temp_path().string() + "/SharedMemory_";
                file_name += framework::simple_version_string();
                file_name += "_";
                file_name += format(iid);
                int fd = ::open(file_name.c_str(), O_CREAT, S_IRWXG | S_IRWXO | S_IRWXU);
                if ( -1 == fd )
                {
                    const char* err_msg = "SystemV share memory create fail!";
                    throw std::runtime_error( err_msg );
                }
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

            shm_t Shm_create( 
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
                    return ( shm_t )-1;
                }

                return ( shm_t )id;
            }

            shm_t Shm_open( 
                boost::uint32_t uni_id,
                boost::uint32_t key,
                boost::system::error_code & ec)
            {
                int id = ::shmget(
                    name_key(uni_id, key), 0, 0666);
                if (id == SHM_NULL) {
                    ec = framework::system::last_system_error();
                    return SHM_NULL;
                }

                return ( shm_t )id;
            }

            void * Shm_map(
                shm_t id,
                boost::system::error_code & ec )
            {
                void * p;
                p = ::shmat(
                    id, 
                    0, 
                    0);
                if (p == (void *)-1) {
                    ec = framework::system::last_system_error();
                    return NULL;
                }
                return p;
            }

            void Shm_unmap( void * addr, size_t size )
            {
                shmdt(addr);
            }

            void Shm_close( shm_t id )
            {
            }

            bool Shm_destory( 
                int uni_id, 
                int key,
                boost::system::error_code & ec)
            {
                int id = ::shmget(
                    name_key(uni_id, key), 0, 0666);
                if (id == SHM_NULL) {
                    ec = framework::system::last_system_error();
                    return false;
                }
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

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_SYSTEM_V_H_
