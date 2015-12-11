// ProcessStatWin32.h

#include "framework/system/LogicError.h"
using namespace framework::system;

#include <windows.h>
#if (!defined UNDER_CE) && (!defined WINRT) && (!defined WIN_PHONE)
#  include <psapi.h>
#  pragma comment(lib, "Psapi.lib")
#endif

namespace framework
{
    namespace process
    {

#if (!defined UNDER_CE) && (!defined WINRT) && (!defined WIN_PHONE)

        boost::system::error_code get_process_environ(
            int pid, 
            std::map<std::string, std::string> & environs)
        {
            return logic_error::not_supported;
        }

        boost::system::error_code get_process_environ(
            int pid, 
            std::string const & key, 
            std::string & value)
        {
            return logic_error::not_supported;
        }
#endif

    } // namespace process
} // namespace framework
