// ConfigModule.cpp

#include "framework/Framework.h"
#include "framework/configure/ConfigModule.h"
#include "framework/configure/Config.h"

namespace framework
{
    namespace configure
    {

        ConfigModule::ConfigModule(
            std::string const & name, 
            Config & conf)
            : name_(name)
              , conf_(conf)
        {
        }

        ConfigModule::~ConfigModule()
        {
            for (iterator ik = begin(); ik != end(); ++ik) {
                (ik->second)->del();
                (ik->second) = NULL;
            }
        }

        boost::system::error_code ConfigModule::set(
            std::string const & key, 
            std::string const & value)
        {
            const_iterator ik = find(key);
            if (ik == end())
                return framework::system::logic_error::item_not_exist;
            return ik->second->set(value);
        }

        boost::system::error_code ConfigModule::get(
            std::string const & key, 
            std::string & value) const
        {
            const_iterator ik = find(key);
            if (ik == end())
                return framework::system::logic_error::item_not_exist;
            return ik->second->get(value);
        }

        boost::system::error_code ConfigModule::get(
            std::map<std::string, std::string> & kvs) const
        {
            for (const_iterator ik = begin(); ik != end(); ++ik) {
                std::string value;
                if (!ik->second->get(value)) {
                    kvs[ik->first] = value;
                }
            }
            return framework::system::logic_error::succeed;
        }

        ConfigModule & ConfigModule::operator()(
            std::string const & key, 
            ConfigItem * item)
        {
            iterator it = find( key );
            if ( it != end() )
                (it->second)->del();

            (*this)[key] = item;

            conf_.register_param(name_, key, item);
            return *this;
        }

    } // namespace configure
} // namespace framework
