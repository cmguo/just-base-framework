// Config.h

/** Config���ڶ�д���ò������Լ���̬��ȡ���������ò���ֵ
* ֧�ֶ����������ͣ�ֻҪParseģ��֧�ָ���������
* ���ò�����ģ����飬��ͬ��Ĳ������ֿ����ظ�ʹ��
* ֧�ַ��ʿ��ƣ��ֱ����ö�д���
* ����ָ���Զ���Ķ�ȡ���������������Զ���ķ�ʽ��Ӧ��д����
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
            // ���ò�����ʼ����������
            typedef void (*initor_type)(
                ConfigItem &, 
                std::string const &);

            // ���ò�����������������
            typedef boost::system::error_code (*settor_type)(
                ConfigItem &, 
                std::string const &);

            // ���ò�����ȡ����������
            typedef boost::system::error_code (*gettor_type)(
                ConfigItem const &, 
                std::string &);

            // ��������������
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

            initor_type initor_; // ��ʼ����
            settor_type settor_; // ������
            gettor_type gettor_; // ��ȡ��
            deletor_type deletor_; // ������
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

        // Ĭ�ϵ�������
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
            // ֱ�ӽ����ַ�������ʼ������ֵ
            static void init(
                ConfigItem & item, 
                std::string const & str)
            {
                C & this_item = static_cast<C &>(item);
                T t;
                if (!framework::string::parse2(str, t))
                    this_item.init_value(t);
            }

            // ֱ�ӽ����ַ������޸Ĳ���ֵ����
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

            // ֱ�ӴӲ���ֵ�����ʽ������ַ���
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

            // ������
            void init_value(
                T const & t)
            {
            }

            // ������
            boost::system::error_code set_value(
                T const & t)
            {
                return framework::system::logic_error::no_permission;
            }

            // ������
            boost::system::error_code get_value(
                T & t) const
            {
                return framework::system::logic_error::no_permission;
            }
        };

        // ������ɱ�־����
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
            // ֱ���޸Ĳ���ֵ����
            void init_value(
                T const & t)
            {
                t_ = t;
            }

            // ֱ���޸Ĳ���ֵ����
            boost::system::error_code set_value(
                T const & t)
            {
                t_ = t;
                return boost::system::error_code();
            }

            // ֱ�ӴӲ���ֵ�����ʽ������ַ���
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
            // ����ֻ���������ļ��ڴ澵��
            Profile & profile()
            {
                return pf_;
            }

        public:
            // ע��һ�����ò�������������ͬһ��ģ��
            ConfigModule & register_module(
                std::string const & module);

            // ע��һ�����ò���
            template <typename T>
            void register_param(
                std::string const & module, 
                std::string const & key, 
                T & t,
                unsigned int flag)
            {
                register_param(module, key, make_item(t, flag));
            }

            // �������ò���ֵ
            boost::system::error_code set(
                std::string const & m, 
                std::string const & k, 
                std::string const & v, 
                bool save = false);

            boost::system::error_code set_force(
                std::string const & m, 
                std::string const & k, 
                std::string const & v);

            // ��ȡ���ò���ֵ
            boost::system::error_code get(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // ��ȡ���ò���ֵ
            boost::system::error_code get_force(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // ��ȡһ��ģ����������ò��������Ƽ���ֵ
            boost::system::error_code get(
                std::string const & m, 
                std::map<std::string, std::string> & kvs);

            // ��ȡ�������ò��������Ƽ���ֵ
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

            // ע��һ�����ò���
            void register_param(
                std::string const & module, 
                std::string const & key, 
                ConfigItem * item) const;

        private:
            Profile pf_; // �����ļ��ڴ澵��
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
