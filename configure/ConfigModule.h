// ConfigModule.h

#ifndef _FRAMEWORK_CONFIGURE_CONFIG_MODULE_H_
#define _FRAMEWORK_CONFIGURE_CONFIG_MODULE_H_

#include "framework/configure/ConfigItem.h"

#include <boost/function.hpp>

namespace framework
{
    namespace configure
    {
        class Config;

        class ConfigModuleBase
        {
        public:
            virtual boost::system::error_code set(
                std::string const & key, 
                std::string const & value) = 0;

            virtual boost::system::error_code get(
                std::string const & key, 
                std::string & value) const = 0;

            virtual boost::system::error_code get(
                std::map<std::string, std::string> & kvs) const = 0;
        };

        class ConfigModule
            : public ConfigModuleBase
            , private std::map<std::string, ConfigItem *>
        {
        public:
            ConfigModule(
                std::string const & name, 
                Config & conf);

            ~ConfigModule();

        public:
            ConfigModule & operator()(
                std::string const & key, 
                ConfigItem * item);

            template <typename T>
            ConfigModule & operator()(
                std::string const & key, 
                T & t, 
                unsigned int flag)
            {
                (*this)(key, make_item(flag, t));
                return *this;
            }

            ConfigModule & operator()(
                std::pair<std::string const, ConfigItem *> key_item)
            {
                (*this)(key_item.first, key_item.second);
                return *this;
            }

            ConfigModule & operator<<(
                std::pair<std::string const, ConfigItem *> key_item)
            {
                (*this)(key_item.first, key_item.second);
                return *this;
            }

            boost::system::error_code set(
                std::string const & key, 
                std::string const & value);

            boost::system::error_code get(
                std::string const & key, 
                std::string & value) const;

            boost::system::error_code get(
                std::map<std::string, std::string> & kvs) const;

        private:
            std::string name_;
            Config const & conf_;
        };

    } // namespace configure
} // namespace framework

#endif // __FRAMEWORK_CONFIGURE_CONFIG_H_
