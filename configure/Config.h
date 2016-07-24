// Config.h

/** Config用于读写配置参数，以及动态获取和重新配置参数值
* 支持多种数据类型，只要Parse模板支持该数据类型
* 配置参数按模块分组，不同组的参数名字可以重复使用
* 支持访问控制，分别设置读写许可
* 可以指定自定义的读取器和设置器，用自定义的方式响应读写请求
*/

#ifndef _FRAMEWORK_CONFIGURE_CONFIG_H_
#define _FRAMEWORK_CONFIGURE_CONFIG_H_

#include "framework/configure/Profile.h"
#include "framework/configure/ConfigModule.h"

#include <boost/function.hpp>

namespace framework
{
    namespace configure
    {

        class Config
            : private std::map<std::string, ConfigModuleBase *>
        {
        public:
            Config();

            Config(
                std::string const & file);

            Config(
                Config const & parent, 
                std::string const & subset);

            ~Config(void);

        public:
            // 返回只读的配置文件内存镜像
            Profile & profile()
            {
                return pf_;
            }

        public:
            // extension config

            typedef boost::function<void (
                std::string const &, 
                std::string const &, 
                std::string const &)
            > set_config_t;

            typedef boost::function<void (
                std::string const &, 
                std::string const &, 
                std::string &)
            > get_config_t;

            void register_ext_config(
                std::string const & key, 
                set_config_t const & set, 
                get_config_t const & get);

            void set_ext_config(
                std::string const & ext, 
                std::string const & sec, 
                std::string const & key, 
                std::string const & value) const;

            void get_ext_config(
                std::string const & ext, 
                std::string const & sec, 
                std::string const & key, 
                std::string & value) const;

        public:
            // 注册一组配置参数，它们属于同一个模块
            ConfigModule & register_module(
                std::string const & module);

            bool register_module(
                std::string const & name, 
                ConfigModuleBase * module);

            // 注册一个配置参数
            template <typename T>
            void register_param(
                std::string const & module, 
                std::string const & key, 
                T & t,
                unsigned int flag)
            {
                register_param(module, key, make_item(t, flag));
            }

            // 设置配置参数值
            boost::system::error_code set(
                std::string const & m, 
                std::string const & k, 
                std::string const & v, 
                bool save = false);

            boost::system::error_code set_force(
                std::string const & m, 
                std::string const & k, 
                std::string const & v, 
                bool save = false);

            // 读取配置参数值
            boost::system::error_code get(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // 读取配置参数值
            boost::system::error_code get_force(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // 获取一个模块的所有配置参数的名称及数值
            boost::system::error_code get(
                std::string const & m, 
                std::map<std::string, std::string> & kvs);

            // 获取所有配置参数的名称及数值
            boost::system::error_code get(
                std::map<std::string, std::map<std::string, std::string> > & mkvs);

            boost::system::error_code sync(
                std::string const & m, 
                std::string const & k);

            boost::system::error_code sync(
                std::string const & m);

            boost::system::error_code sync();

        private:
            friend class ConfigModule;

            // 注册一个配置参数
            void register_param(
                std::string const & module, 
                std::string const & key, 
                ConfigItem * item) const;

        private:
            Profile pf_; // 配置文件内存镜像
            typedef std::map<std::string, std::pair<
                set_config_t, get_config_t>
            > ext_config_map_t;
            ext_config_map_t ext_configs_;
        };

    } // namespace configure
} // namespace framework

#endif // _FRAMEWORK_CONFIGURE_CONFIG_H_
