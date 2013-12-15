// Convert.cpp

#include "framework/Framework.h"
#include "framework/string/Convert.h"
#include "framework/system/ErrorCode.h"
#include "framework/system/LogicError.h"
using namespace framework::system::logic_error;

#ifdef FRAMEWORK_WITH_LIB_ICONV
#  ifndef BOOST_WINDOWS_API
#    include <framework/string/impl/ConvertLinux.h>
#  else
#    include <framework/string/impl/ConvertWin32.h>
#  endif
#else
#  include <framework/string/impl/ConvertSim.h>
#endif

namespace framework
{
    namespace string
    {

        Convert::Convert()
            : ctx_(NULL)
            , err_pos_(0)
        {
        }

        Convert::Convert(
            std::string const & t1, 
            std::string const & t2)
            : ctx_(NULL)
            , err_pos_(0)
        {
            boost::system::error_code ec;
            open(t1, t2, ec);
        }

        Convert::~Convert()
        {
            close();
        }

    } // namespace string
} // namespace framework
