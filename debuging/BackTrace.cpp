// MemoryPoolDebug.cpp

#include "framework/Framework.h"

#ifndef FRAMEWORK_WITH_BACKTRACE
#  include <framework/debuging/impl/BackTraceSim.h>
#else
#  ifdef BOOST_WINDOWS_API
#    include <framework/debuging/impl/BackTraceWin32.h>
#  else
#    include <framework/debuging/impl/BackTraceLinux.h>
#  endif
#endif
