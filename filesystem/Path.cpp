// Path.cpp

#include "framework/Framework.h"
#include "framework/filesystem/Path.h"
#include "framework/process/Environments.h"
#include "framework/filesystem/Symlink.h"
#include "framework/string/Slice.h"
#include "framework/Version.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <boost/interprocess/shared_memory_object.hpp>
#endif

namespace framework
{
    namespace filesystem
    {

        static size_t const MAX_PATH_SIZE = 1024;

        /// 获取当前可执行文件路径
        /// 
        ///     @note linux下为用户shell的路径
        boost::filesystem::path bin_file()
        {
#ifdef BOOST_WINDOWS_API
            char path[MAX_PATH_SIZE] = { 0 };
            DWORD len = ::GetModuleFileNameA(NULL, path, MAX_PATH_SIZE);
            if (0 != len) {
                return path;
            } else {
                assert(!"GetModuleFileName failed");
                return boost::filesystem::path();
            }
#else

#ifdef __FreeBSD__
#  define PROC_EXE "file"
#else
#  define PROC_EXE "exe"
#endif
            boost::system::error_code ec;
            boost::filesystem::path ph_h( "/proc/self" );
            boost::filesystem::path ph(boost::filesystem::read_symlink(ph_h / PROC_EXE , ec));
            return ph;
#endif
        }

        boost::filesystem::path bin_path()
        {
            return bin_file().remove_leaf();
        }

        /// 获取临时文件的路径
        ///
        ///     @note linux下一般为：/tmp下
        boost::filesystem::path temp_path()
        {
#ifdef BOOST_WINDOWS_API
            char path[MAX_PATH_SIZE] = { 0 };
            DWORD len = ::GetTempPathA(MAX_PATH_SIZE, path);
            if (0 != len) {
                return path;
            } else {
                assert(!"GetTempPath failed");
                return boost::filesystem::path();
            }
#else
            boost::system::error_code ec;
            return boost::filesystem::temp_directory_path(ec);
#endif
        }

        /// 获取framework临时文件存放的路径
        ///
        ///     @note linux下一般为：/tmp下
        boost::filesystem::path framework_temp_path()
        {
            boost::filesystem::path framework_tmp_dir = temp_path();
            try {
                std::string dirname = "framework_";
                dirname += framework::version_string();
                framework_tmp_dir /= boost::filesystem::path( dirname );
                boost::filesystem::create_directories(framework_tmp_dir);
            } catch ( ... ) {
                // do nothing here
            }
            return framework_tmp_dir;
        }

        /// 获取日志路径
        ///
        ///     @note 临时文件的路径相同
        boost::filesystem::path log_path()
        {
            return temp_path();
        }

        /// 查找可执行文件所在的路径
        boost::filesystem::path which_bin_file(
            boost::filesystem::path const & file);

        /// 查找配置文件所在的路径
        ///
        ///     在环境变量LD_CONFIG_PATH的路径下查找配置文件并返回，否则返回本文件名。
        ///     @note linux下还继续在 /etc 下进行配置文件的查找
        boost::filesystem::path which_config_file(
            boost::filesystem::path const & file)
        {
            /// 绝对路径直接返回
            if ( file.has_root_directory() ) return file;

            std::vector<boost::filesystem::path> search_paths;
            search_paths.push_back(".");

            std::vector<std::string> env_config_paths;
            std::string env_config_paths_str = 
                framework::process::get_environment("LD_CONFIG_PATH");
#ifdef BOOST_WINDOWS_API
#  define DELIM ";"
#else
#  define DELIM ":"
#endif
            framework::string::slice<std::string>(
                env_config_paths_str, std::back_inserter(env_config_paths), DELIM);
#undef DELIM

            search_paths.insert(search_paths.end(), env_config_paths.begin(), env_config_paths.end());
#ifndef BOOST_WINDOWS_API
            env_config_paths.push_back("/etc");
#endif
            search_paths.push_back(temp_path());
            search_paths.push_back(bin_path());

            for (size_t i = 0; i < search_paths.size(); ++i) {
                boost::filesystem::path ph(search_paths[i] / file);
                if (boost::filesystem::exists(ph))
                    return ph;
            }
            return file;
        }

    } // namespace filesystem
} // namespace framework
