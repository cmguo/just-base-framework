// Logger.cpp

#include "framework/Framework.h"
#include "framework/logger/Logger.h"
#include "framework/logger/Manager.h"

namespace framework
{
    namespace logger
    {

        void load_config(
            Manager & mgr, 
            framework::configure::Config & conf)
        {
            mgr.load_config(conf);
        }

        Module & register_module(
            Manager & mgr, 
            char const * name, 
            size_t level)
        {
            return mgr.register_module(name, level);
        }

        bool add_stream(
            Manager & mgr, 
            Stream & s)
        {
            return mgr.add_stream(s);
        }

        bool add_stream(
            Manager & mgr, 
            Stream & s, 
            std::string const & group)
        {
            return mgr.add_stream(s, group);
        }

        bool del_stream(
            Manager & mgr, 
            Stream & s)
        {
            return mgr.del_stream(s);
        }


    } // namespace logger
} // namespace framework
