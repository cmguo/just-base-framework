// Library.cpp

#include "framework/Framework.h"
#include "framework/library/Library.h"
#include "framework/system/ErrorCode.h"
#include "framework/logger/Logger.h"
#include "framework/logger/FormatRecord.h"
#include "framework/process/Environments.h"
#include "framework/filesystem/Path.h"
#include "framework/string/Slice.h"
using namespace framework::system;

#include <boost/filesystem/operations.hpp>
using namespace boost::system;

#include <iostream>

#ifdef BOOST_WINDOWS_API
#include <windows.h>
#else
#include <dlfcn.h>
#endif

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("framework.library.Library", framework::logger::Warn)

namespace framework
{
    namespace library
    {
   
        Library::Library()
            : handle_(NULL)
            , need_close_(false)
        {
        }

        Library::Library(
            Library const & r)
            : handle_(r.handle_)
            , need_close_(false)
        {
        }

        Library::Library(
            std::string const & name)
            : handle_(NULL)
            , need_close_(false)
        {
            open(name);
        }

        Library::Library(
            void * handle)
            : handle_(handle)
            , need_close_(false)
        {
        }

        Library::~Library()
        {
            if (handle_ && need_close_)
                close();
        }

        error_code Library::open(
            std::string const & name)
        {
#ifdef BOOST_WINDOWS_API
            handle_ = ::LoadLibraryA(name.c_str());
			if (GetLastError() == ERROR_MOD_NOT_FOUND) {
				SetLastError(ERROR_PATH_NOT_FOUND);
			}
#else
            std::string name2;
            if (name.find('.') == std::string::npos) {
                name2 = std::string("lib") + name + ".so";
            } else {
                name2 = name;
            }
            handle_ = ::dlopen(name2.c_str(), RTLD_LAZY | RTLD_LOCAL);
#ifdef __ANDROID__
            if (handle_ == NULL) {
                std::vector<std::string> env_config_paths;
                std::string env_config_paths_str = 
                    framework::process::get_environment("LD_LIBRARY_PATH");
                framework::string::slice<std::string>(
                    env_config_paths_str, std::back_inserter(env_config_paths), ":");
                for (size_t i = 0; i < env_config_paths.size(); ++i) {
                    boost::filesystem::path ph(env_config_paths[i]);
                    ph /= name2;
                    if (boost::filesystem::exists(ph)) {
                        handle_ = ::dlopen(ph.string().c_str(), RTLD_LAZY | RTLD_LOCAL);
                        break;
                    }
                }
            }
#endif
            if (handle_ == NULL)
                LOG_WARN("[open] dlopen: %1%" % ::dlerror());
#endif
            if (handle_)
                need_close_ = true;
            return handle_ ? error_code() : last_system_error();
        }

        error_code Library::close()
        {
#ifdef BOOST_WINDOWS_API
            ::FreeLibrary((HMODULE)handle_);
#else
            ::dlclose(handle_);
#endif
            handle_ = NULL;
            return last_system_error();
        }

        void * Library::symbol(
            std::string const & name) const
        {
#ifdef BOOST_WINDOWS_API
            return (void *)::GetProcAddress((HMODULE)handle_, name.c_str());
#else
            return ::dlsym(handle_, name.c_str());
#endif
        }

        static size_t const MAX_PATH_SIZE = 1024;

        std::string Library::path() const
        {
#ifdef BOOST_WINDOWS_API
            char path[MAX_PATH_SIZE] = {0};
            DWORD len = ::GetModuleFileNameA((HMODULE)handle_, path, MAX_PATH_SIZE);
            if (0 != len) {
                return path;
            }
#elif __USE_GNU
            Dl_info info;
            int ret = ::dladdr(handle_, &info);
            if (ret != 0) {
                return info.dli_fname;
            }
#endif
            return std::string();
        }

        Library Library::self()
        {
            return from_address((void *)&Library::self);
        }

        Library Library::from_address(
            void * addr)
        {
#ifdef BOOST_WINDOWS_API
            MEMORY_BASIC_INFORMATION info = {0};
            if (sizeof(info) != ::VirtualQuery(addr, &info, sizeof(info))) {
                return info.AllocationBase;
            }
#elif __USE_GNU
            Dl_info info;
            int ret = ::dladdr(addr, &info);
            if (ret != 0) {
                return info.dli_fbase;
            }
#endif
            return NULL;
        }

    } // namespace library
} // namespace framework
