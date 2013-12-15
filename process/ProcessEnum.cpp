// ProcessEnum.cpp

#include "framework/Framework.h"
#include "framework/process/ProcessEnum.h"
#include "framework/process/Error.h"
#include "framework/system/ErrorCode.h"
using namespace framework::system;

#include <boost/filesystem/operations.hpp>
using namespace boost::filesystem;
using namespace boost::system;

#ifdef BOOST_WINDOWS_API
#  include "framework/process/impl/ProcessEnumWin32.h"
#else
#  include "framework/process/impl/ProcessEnumLinux.h"
#endif
