// Logger.h

#ifndef _FRAMEWORK_LOGGER_LOGGER_H_
#define _FRAMEWORK_LOGGER_LOGGER_H_

#include "framework/logger/Level.h"
#include "framework/logger/Module.h"
#include "framework/logger/Record.h"

namespace framework
{
    namespace configure
    {
        class Config;
    }

    namespace logger
    {

        class Manager;
        class Stream;

        Manager & global_logger();

        inline static Manager & _slog(...)
        {
            return global_logger();
        }

        void load_config(
            Manager & mgr, 
            framework::configure::Config & conf);

        Module & register_module(
            Manager & mgr, 
            char const * name, 
            size_t level = Warn);

        bool add_stream(
            Manager & mgr, 
            Stream & s);

        bool add_stream(
            Manager & mgr, 
            Stream & s, 
            std::string const & group);

        bool del_stream(
            Manager & mgr, 
            Stream & s);

        void load_config(
            framework::configure::Config & conf)
        {
            load_config(global_logger(), conf);
        }

        Module & register_module(
            char const * name, 
            size_t level = Warn)
        {
            return register_module(global_logger(), name, level);
        }

        bool add_stream(
            Stream & s)
        {
            return add_stream(global_logger(), s);
        }

        bool add_stream(
            Stream & s, 
            std::string const & group)
        {
            return add_stream(global_logger(), s, group);
        }

        bool del_stream(
            Stream & s)
        {
            return del_stream(global_logger(), s);
        }

        static inline void log(
            Module const & module, 
            size_t level, 
            Record const & record)
        {
            module.log(level, record);
        }

    } // namespace logger
} // namespace framework

#define FRAMEWORK_LOGGER_DECLARE_LOGGER(name_) \
    inline static framework::logger::Manager & _slog() \
{ \
    static framework::logger::Manager logmgr; \
    return logmgr; \
}

/// 注册一个模块，默认等级为6，插入到默认日志模块
#define FRAMEWORK_LOGGER_DECLARE_MODULE(name_) \
    inline static framework::logger::Module const & _slogm() \
{ \
    using framework::logger::_slog; \
    static framework::logger::Module const & module = \
        framework::logger::register_module(_slog(), name_); \
    return module; \
}

/// 注册一个模块，指定模块等级，插入到默认日志模块
#define FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL(name_, level) \
    inline static framework::logger::Module const & _slogm() \
{ \
    using framework::logger::_slog; \
    static framework::logger::Module const & module = \
    framework::logger::register_module(_slog(), name_, level); \
    return module; \
}

#define FRAMEWORK_LOGGER_DECLARE_MODULE_USE_BASE(base) \
    using base::_slogm

#endif // _FRAMEWORK_LOGGER_LOGGER_H_
