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

        /// ��ȡ��ǰ��ִ���ļ�·��
        /// 
        ///     @note linux��Ϊ�û�shell��·��
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
            boost::filesystem::path ph(read_symlink(ph_h / PROC_EXE , ec));
            return ph;
#endif
        }

        boost::filesystem::path bin_path()
        {
            return bin_file().remove_leaf();
        }

        /// ��ȡ��ʱ�ļ���·��
        ///
        ///     @note linux��һ��Ϊ��/tmp��
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
            return boost::interprocess::detail::get_temporary_path();
#endif
        }

        /// ��ȡframework��ʱ�ļ���ŵ�·��
        ///
        ///     @note linux��һ��Ϊ��/tmp��
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

        /// ��ȡ��־·��
        ///
        ///     @note ��ʱ�ļ���·����ͬ
        boost::filesystem::path log_path()
        {
            return temp_path();
        }

        /// ���ҿ�ִ���ļ����ڵ�·��
        boost::filesystem::path which_bin_file(
            boost::filesystem::path const & file);

        /// ���������ļ����ڵ�·��
        ///
        ///     �ڻ�������LD_CONFIG_PATH��·���²��������ļ������أ����򷵻ر��ļ�����
        ///     @note linux�»������� /etc �½��������ļ��Ĳ���
        boost::filesystem::path which_config_file(
            boost::filesystem::path const & file)
        {
            /// ����·��ֱ�ӷ���
            if ( file.has_root_directory() ) return file;

            std::vector<boost::filesystem::path> search_paths;

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
