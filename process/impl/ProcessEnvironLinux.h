// Process.cpp

#include "framework/string/Format.h"
#include "framework/system/LogicError.h"
using namespace framework::system;

#include <boost/filesystem/path.hpp>
using namespace boost::filesystem;

#include <fstream>
#include <algorithm>

namespace framework
{
    namespace process
    {

        boost::system::error_code get_process_environ(
            int pid, 
            std::map<std::string, std::string> & environs)
        {
            path ph(path("/proc") / framework::string::format(pid) / "environiron");
            std::ifstream ifs(ph.string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            std::string key, value;
            while (!ifs.eof()) {
                ifs >> key;
                ifs.ignore(1);
                std::getline(ifs, value, '\0');
                if (!key.empty())
                    environs[key] = value;
            }
            return boost::system::error_code();
        }

        boost::system::error_code get_process_environ(
            int pid, 
            std::string const & key, 
            std::string & value)
        {
            path ph(path("/proc") / framework::string::format(pid) / "environiron");
            std::ifstream ifs(ph.string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            std::string key1, value1;
            while (!ifs.eof()) {
                ifs >> key1;
                ifs.ignore(1);
                std::getline(ifs, value1, '\0');
                if (key1 == key) {
                    value = value1;
                    return boost::system::error_code();
                }
            }
            return logic_error::item_not_exist;
        }

    } // namespace process
} // namespace framework
