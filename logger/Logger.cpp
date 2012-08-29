// Logger.cpp

#include "framework/Framework.h"
#include "framework/logger/Logger.h"
#include "framework/logger/Manager.h"

namespace framework
{
    namespace logger
    {

        Module & register_module(
            Manager & mgr, 
            char const * name, 
            size_t level)
        {
            return mgr.register_module(name, level);
        }

    } // namespace logger
} // namespace framework
