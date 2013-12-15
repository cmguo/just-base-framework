// Process.cpp

#include "framework/Framework.h"
#include "framework/process/ProcessStat.h"
#include "framework/system/ErrorCode.h"
using namespace framework::system;

#ifdef BOOST_WINDOWS_API
#  include "framework/process/impl/ProcessStatWin32.h"
#else
#  include "framework/process/impl/ProcessStatLinux.h"
#endif
