// Config.cpp

#include "framework/Framework.h"
#include "framework/configure/Config.h"
#include "framework/filesystem/Path.h"

namespace framework
{
    namespace configure
    {

        using boost::system::error_code;
        using framework::system::logic_error::succeed;
        using framework::system::logic_error::item_not_exist;
        using framework::system::logic_error::no_permission;

        Config::Config()
        {
            boost::filesystem::path ph = framework::filesystem::bin_file();
            ph.replace_extension(".ini");
            pf_.load(ph.string());
        }

        Config::Config(
            std::string const & file)
            : pf_(framework::filesystem::which_config_file(file).string())
        {
        }

        Config::Config(
            Config const & parent, 
            std::string const & subset)
            : pf_(parent.pf_, subset)
        {
        }

        Config::~Config(void)
        {
        }

        void Config::register_ext_config(
            std::string const & key, 
            set_config_t const & set, 
            get_config_t const & get)
        {
            boost::mutex::scoped_lock lock(mutex_);
            ExtConfig & ext = ext_configs_[key];
            ext.set = set;
            ext.get = get;
            for (mkv_map_t::const_iterator iter = ext.preset.begin(); iter != ext.preset.end(); ++iter) {
                for (kv_map_t::const_iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2) {
                    ext.set(iter->first, iter2->first, iter2->second);
                }
            }
        }

        void Config::set_ext_config(
            std::string const & ext, 
            std::string const & sec, 
            std::string const & key, 
            std::string const & value)
        {
            boost::mutex::scoped_lock lock(mutex_);
            ExtConfig & extcfg = ext_configs_[ext];
            if (extcfg.set.empty()) {
                extcfg.preset[sec][key] = value;
            } else {
                extcfg.set(sec, key, value);
            }
        }

        void Config::get_ext_config(
            std::string const & ext, 
            std::string const & sec, 
            std::string const & key, 
            std::string & value)
        {
            boost::mutex::scoped_lock lock(mutex_);
            ext_config_map_t::const_iterator iter = 
                ext_configs_.find(ext);
            if (iter != ext_configs_.end()) {
                iter->second.get(sec, key, value);
            }
        }

        ConfigModule & Config::register_module(
            std::string const & module)
        {
            boost::mutex::scoped_lock lock(mutex_);
            iterator im = find(module);
            if (im == end())
                im = insert(std::make_pair(module, new ConfigModule(module, *this))).first;
            return *static_cast<ConfigModule *>(im->second);
        }

        bool Config::register_module(
            std::string const & name, 
            ConfigModuleBase * module)
        {
            boost::mutex::scoped_lock lock(mutex_);
            iterator im = find(name);
            if (im == end()) {
                insert(std::make_pair(name, module));
                return true;
            } else {
                return false;
            }
        }

        void Config::register_param(
            std::string const & module, 
            std::string const & key, 
            ConfigItem * item)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::string value;
            if (pf_.get_inherit(module, key, value) == 0) {
                item->init(value);
            }
        }

        error_code Config::set(
            std::string const & m, 
            std::string const & k, 
            std::string const & v, 
            bool save)
        {
            boost::mutex::scoped_lock lock(mutex_);
            iterator im = find(m);
            if (im == end())
                return error_code(item_not_exist);
            error_code ec = im->second->set(k, v);
            if (!ec)
                pf_.set(m, k, v, save);
            return ec;
        }


        error_code Config::set_force(
            std::string const & m, 
            std::string const & k, 
            std::string const & v, 
            bool save)
        {
            error_code ec = set(m, k, v, save);
            if (ec == no_permission || ec == item_not_exist) {
                // 即使不存在，或者没权限，也让设置
                boost::mutex::scoped_lock lock(mutex_);
                pf_.set(m, k, v, save);
                //ec = success;
            }
            return ec;
        }

        error_code Config::get(
            std::string const & m, 
            std::string const & k, 
            std::string & v)
        {
            boost::mutex::scoped_lock lock(mutex_);
            const_iterator im = find(m);
            if (im == end())
                return item_not_exist;
            return im->second->get(k, v);
        }

        error_code Config::get_force(
            std::string const & m, 
            std::string const & k, 
            std::string & v)
        {
            error_code ec = get(m, k, v);
            if (ec == no_permission || ec == item_not_exist) {
                boost::mutex::scoped_lock lock(mutex_);
                pf_.get(m, k, v);
            }
            return ec;
        }

        error_code Config::get(
            std::string const & m, 
            kv_map_t & kvs)
        {
            boost::mutex::scoped_lock lock(mutex_);
            const_iterator im = find(m);
            if (im == end())
                return item_not_exist;
            return im->second->get(kvs);
        }

        error_code Config::get(
            mkv_map_t & mkvs)
        {
            boost::mutex::scoped_lock lock(mutex_);
            const_iterator im = begin();
            for (; im != end(); ++im) {
                kv_map_t & kvs = mkvs[im->first];
                im->second->get(kvs);
            }
            return succeed;
        }

        error_code Config::sync(
            std::string const & m, 
            std::string const & k)
        {
            boost::mutex::scoped_lock lock(mutex_);
            std::string v;
            error_code ec = get(m, k, v);
            if (ec)
                return ec;
            pf_.set(m, k, v);
            return succeed;
        }

        boost::system::error_code Config::sync(
            std::string const & m)
        {
            kv_map_t kvs;
            boost::mutex::scoped_lock lock(mutex_);
            error_code ec = get(m, kvs);
            if (ec)
                return ec;
            for (kv_map_t::const_iterator iter = kvs.begin(); iter != kvs.end(); ++iter) {
                pf_.set(m, iter->first, iter->second, false);
            }
            pf_.save();
            return succeed;
        }

        boost::system::error_code Config::sync()
        {
            boost::mutex::scoped_lock lock(mutex_);
            const_iterator im = begin();
            for (; im != end(); ++im) {
                kv_map_t kvs;
                im->second->get(kvs);
                for (kv_map_t::const_iterator iter = kvs.begin(); iter != kvs.end(); ++iter) {
                    pf_.set(im->first, iter->first, iter->second, false);
                }
            }
            pf_.save();
            return succeed;
        }

    } // namespace configure
} // namespace framework
