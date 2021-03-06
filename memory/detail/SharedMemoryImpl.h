// SharedMemoryImpl.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_IMPL_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_IMPL_H_

#include "framework/Version.h"
#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"
#include "framework/filesystem/Path.h"

namespace framework
{
    namespace memory
    {

        namespace detail
        {

            typedef intptr_t map_id_t;

            class ObjectWrapper
            {
            public:
                ObjectWrapper()
                    : obj_(0)
                    , close_func_(0)
                    , close_object_(NULL)
                {
                }

                ~ObjectWrapper()
                {
                    if (close_object_) {
                        reset();
                    }
                }

                template <
                    typename Object, 
                    typename CloseFunc
                >
                void reset(
                    Object obj, 
                    CloseFunc close_func)
                {
                    assert(close_object_ == NULL);
                    obj_ = (map_id_t)obj;
                    close_func_ = (map_id_t)close_func;
                    close_object_ = &ObjectWrapper::close_object<Object, CloseFunc>;
                }

                void reset()
                {
                    close_object_(obj_, close_func_);
                    close_object_ = NULL;
                }

                map_id_t release()
                {
                    close_object_ = NULL;
                    return obj_;
                }

                template <
                    typename Object
                >
                static Object cast_object(
                    map_id_t obj)
                {
                    return (Object)obj;
                }

            private:
                template <
                    typename Object, 
                    typename CloseFunc
                >
                static void close_object(
                    map_id_t obj, 
                    map_id_t func)
                {
                    ((CloseFunc)func)((Object)obj);
                }

            private:
                map_id_t obj_;
                map_id_t close_func_;
                void (* close_object_)(map_id_t, map_id_t);
            };

            class ErrorCodeWrapper
            {
            public:
                ErrorCodeWrapper(
                    boost::system::error_code & ec)
                    : ec_(ec)
                {
                }

                ~ErrorCodeWrapper()
                {
                    ec_ = framework::system::last_system_error();
                }

            private:
                boost::system::error_code & ec_;
            };

            class SharedMemoryImpl
            {
            public:
                virtual ~SharedMemoryImpl() {}

            public:
                virtual bool create(
                    map_id_t* id, 
                    boost::uint32_t iid, 
                    boost::uint32_t key, 
                    boost::uint32_t size,
                    boost::system::error_code & ec) = 0;

                virtual bool open(
                    map_id_t* id, 
                    boost::uint32_t iid, 
                    boost::uint32_t key, 
                    boost::system::error_code & ec) = 0;

                virtual bool close(
                    map_id_t id, 
                    boost::system::error_code & ec) = 0;

                virtual void * map(
                    map_id_t id, 
                    boost::uint32_t size,
                    boost::system::error_code & ec) = 0;

                virtual bool unmap(
                    void * addr, 
                    boost::uint32_t size, 
                    boost::system::error_code & ec) = 0;

                virtual bool destory(
                    boost::uint32_t iid, 
                    boost::uint32_t key, 
                    boost::system::error_code & ec) = 0;

            protected:
                static std::string key_file(
                    boost::uint32_t iid, 
                    boost::uint32_t key)
                {
                    std::string file = "SharedMemory_";
                    file += framework::simple_version_string();
                    file += "_";
                    file += format(iid);
                    file += "_" + format( key );
                    return file;
                }

                static std::string key_path(
                    boost::uint32_t iid, 
                    boost::uint32_t key)
                {
                    std::string path = framework::filesystem::framework_temp_path().string();
                    path += "/";
                    path += SharedMemoryImpl::key_file(iid, key);
                    return path;
                }

            };

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_SYSTEMV_H_
