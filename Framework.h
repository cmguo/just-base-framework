#ifndef _FRAMEWORK_H_
#define _FRAMEWORK_H_

// »ù±¾¿â
#include <time.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <deque>
#include <stack>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#pragma warning( disable : 4290 )
#pragma warning( disable : 4819 )
#endif

#include <boost/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp>

namespace boost
{
    namespace asio
    {
        class io_service;
    }
}

#ifdef FRAMEWORK_USER_CONFIG
#include FRAMEWORK_USER_CONFIG
#endif

namespace framework
{
    namespace configure
    {
        class Config;
    }
}

#include "framework/system/LogicError.h"

#endif
