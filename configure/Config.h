// Config.h

/** Config用于读写配置参数，以及动态获取和重新配置参数值
* 支持多种数据类型，只要Parse模板支持该数据类型
* 配置参数按模块分组，不同组的参数名字可以重复使用
* 支持访问控制，分别设置读写许可
* 可以指定自定义的读取器和设置器，用自定义的方式响应读写请求
*/

#ifndef __FRAMEWORK_CONFIGURE_CONFIG_H_
#define __FRAMEWORK_CONFIGURE_CONFIG_H_

#include "framework/configure/Profile.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/system/LogicError.h"


//#include <boost/function.hpp>

namespace framework
{
    namespace configure
    {
        class Config;

        class ConfigItem
        {
        public:
            // 配置参数初始化函数类型
            typedef void (*initor_type)(
                ConfigItem &, 
                std::string const &);

            // 配置参数设置器函数类型
            typedef boost::system::error_code (*settor_type)(
                ConfigItem &, 
                std::string const &);

            // 配置参数读取器函数类型
            typedef boost::system::error_code (*gettor_type)(
                ConfigItem const &, 
                std::string &);

            // 销毁器函数类型
            typedef void (*deletor_type)(
                ConfigItem &);

        protected:
            ConfigItem(
                unsigned int flag = 0)
                : flag_(flag)
                , settor_(NULL)
                , gettor_(NULL)
            {
            }

            ConfigItem(
                unsigned int flag, 
                initor_type initor, 
                settor_type settor, 
                gettor_type gettor, 
                deletor_type deletor)
                : flag_(flag)
                , initor_(initor)
                , settor_(settor)
                , gettor_(gettor)
                , deletor_(deletor)
            {
            }

        private:
            // non copyable
            ConfigItem(
                ConfigItem const & r);

            ConfigItem & operator=(
                ConfigItem const & r);

        public:
            void init(
                std::string const & str)
            {
                initor_(*this, str);
            }

            boost::system::error_code set(
                std::string const & str)
            {
                return settor_(*this, str);
            }

            boost::system::error_code get(
                std::string & str) const
            {
                return gettor_(*this, str);
            }

            void del()
            {
                deletor_(*this);
            }

            unsigned int flag_;

            initor_type initor_; // 初始化器
            settor_type settor_; // 设置器
            gettor_type gettor_; // 读取器
            deletor_type deletor_; // 销毁器
        };

        class ConfigModule
            : private std::map<std::string, ConfigItem *>
        {
        public:
            ConfigModule(
                std::string const & name, 
                Config & conf)
                : name_(name)
                , conf_(conf)
            {
            }

            ~ConfigModule()
            {
                for (iterator ik = begin(); ik != end(); ++ik) {
                    (ik->second)->del();
                    (ik->second) = NULL;
                }
            }

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
                std::string const & value)
            {
                const_iterator ik = find(key);
                if (ik == end())
                    return framework::system::logic_error::item_not_exist;
                return ik->second->set(value);
            }

            boost::system::error_code get(
                std::string const & key, 
                std::string & value) const
            {
                const_iterator ik = find(key);
                if (ik == end())
                    return framework::system::logic_error::item_not_exist;
                return ik->second->get(value);
            }

            boost::system::error_code get(
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

        private:
            std::string name_;
            Config const & conf_;
        };

        // 默认的配置器
        template <typename T, typename C>
        struct ConfigItemHelper
            : public ConfigItem
        {
            ConfigItemHelper( unsigned int flag = 0)
                : ConfigItem(
                    flag,
                    C::init,
                    C::set, 
                    C::get, 
                    C::del)
            {
            }

        private:
            // 直接解析字符串并初始化参数值
            static void init(
                ConfigItem & item, 
                std::string const & str)
            {
                C & this_item = static_cast<C &>(item);
                T t;
                if (!framework::string::parse2(str, t))
                    this_item.init_value(t);
            }

            // 直接解析字符串并修改参数值本身
            static boost::system::error_code set(
                ConfigItem & item, 
                std::string const & str)
            {
                C & this_item = static_cast<C &>(item);
                T t;
                boost::system::error_code ec = framework::string::parse2(str, t);
                if (!ec)
                    ec = this_item.set_value(t);
                return ec;
            }

            // 直接从参数值本身格式化结果字符串
            static boost::system::error_code get(
                ConfigItem const & item, 
                std::string & str)
            {
                C const & this_item = static_cast<C const &>(item);
                T t;
                boost::system::error_code ec = this_item.get_value(t);
                if (!ec)
                    ec = framework::string::format2(str, t);
                return ec;
            }

            static void del(
                ConfigItem & item)
            {
                C & this_item = static_cast<C &>(item);
                delete &this_item;
            }

            // 不允许
            void init_value(
                T const & t)
            {
            }

            // 不允许
            boost::system::error_code set_value(
                T const & t)
            {
                return framework::system::logic_error::no_permission;
            }

            // 不允许
            boost::system::error_code get_value(
                T & t) const
            {
                return framework::system::logic_error::no_permission;
            }
        };

        // 访问许可标志定义
        enum Flag {
            allow_set = 1, 
            allow_get = 2, 
        };

        template <typename T>
        struct ConfigItemT
            : public ConfigItemHelper<T, ConfigItemT< T > >
        {
        public:
            ConfigItemT(
                T & t, 
                unsigned int flag)
                : ConfigItemHelper<T, ConfigItemT>( flag )
                , t_(t)
            {
            }

        public:
            // 直接修改参数值本身
            void init_value(
                T const & t)
            {
                t_ = t;
            }

            // 直接修改参数值本身
            boost::system::error_code set_value(
                T const & t)
            {
                t_ = t;
                return boost::system::error_code();
            }

            // 直接从参数值本身格式化结果字符串
            boost::system::error_code get_value(
                T & t) const
            {
                t =  t_;
                return boost::system::error_code();
            }

        private:
            T & t_;
        };

        template <typename T>
        static ConfigItemT<T> * make_item(
            T & t, 
            unsigned int flag)
        {
            return new ConfigItemT<T>(t, flag);
        }

        class Config
            : private std::map<std::string, ConfigModule>
        {
        public:
            Config();

            Config(
                std::string const & file);

            ~Config(void);

        public:
            // 返回只读的配置文件内存镜像
            Profile & profile()
            {
                return pf_;
            }

        public:
            // 注册一组配置参数，它们属于同一个模块
            ConfigModule & register_module(
                std::string const & module);

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
                std::string const & v);

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
        };

        inline ConfigModule & ConfigModule::operator()(
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

#define CONFIG_PARAM(p, flag) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (flag)) \
    )

#define CONFIG_PARAM_NOACC(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (0)) \
    )

#define CONFIG_PARAM_RDONLY(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (framework::configure::allow_get)) \
    )

#define CONFIG_PARAM_WRONLY(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (framework::configure::allow_set)) \
    )

#define CONFIG_PARAM_RDWR(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, \
    (framework::configure::allow_get | framework::configure::allow_set)) \
    )

#define CONFIG_PARAM_NAME(name, p, flag) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (flag)) \
    )

#define CONFIG_PARAM_NAME_NOACC(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (0)) \
    )

#define CONFIG_PARAM_NAME_RDONLY(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (framework::configure::allow_get)) \
    )

#define CONFIG_PARAM_NAME_WRONLY(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (framework::configure::allow_set)) \
    )

#define CONFIG_PARAM_NAME_RDWR(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, \
    (framework::configure::allow_get | framework::configure::allow_set)) \
    )


#endif // __FRAMEWORK_CONFIGURE_CONFIG_H_
