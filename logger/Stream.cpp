// Stream.cpp

#include "framework/Framework.h"
#include "framework/logger/Stream.h"
#include "framework/configure/Config.h"
using namespace framework::configure;

namespace framework
{
    namespace logger
    {

        static std::map<std::string, Stream * (*)()> & creator_map()
        {
            static std::map<std::string, Stream * (*)()> cmap;
            return cmap;
        }

        bool Stream::register_(
            std::string const & type, 
            Stream * (* creator)())
        {
            creator_map()[type] = creator;
            return true;
        }

        Stream * Stream::create(
            ConfigModule & cm)
        {
            std::string type("file");
            cm << CONFIG_PARAM_NOACC(type);
            std::map<std::string, Stream * (*)()>::const_iterator iter = creator_map().find(type);
            if (iter == creator_map().end())
                return NULL;
            Stream * s = iter->second();
            s->load_config(cm);
            return s;
        }

        Stream::Stream()
            : next_(NULL)
            , level_(0)
        {
        }

        void Stream::load_config(
            ConfigModule & cm)
        {
            cm << CONFIG_PARAM_NAME_RDWR("level", level_);
        }

    } //namespace logger
} // namespace framework
