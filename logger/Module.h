// Module.h

#ifndef _FRAMEWORK_LOGGER_MODULE_H_
#define _FRAMEWORK_LOGGER_MODULE_H_

#include "framework/logger/GroupBase.h"
#include "framework/logger/Record.h"

namespace framework
{
    namespace logger
    {

        class Manager;
        class Group;
        class Context;

        /// 模块定义
        class Module
        {
        public:
            Module(
                GroupBase * group, 
                char const * name, 
                size_t level = 0);

        public:
            void log(
                size_t level, 
                Record const & record) const
            {
                // 多线程函数静态变量初始化有冲突，可能为空指针
                if (this == NULL)
                    return;
                /// 进行流最大等级过滤
                if (level > level_ 
                    || level > group_->level())
                    return;
                log_it(level, record);

            }

            bool log_accept(
                size_t level) const
            {
                // 多线程函数静态变量初始化有冲突，可能为空指针
                if (this == NULL)
                    return false;
                /// 进行流最大等级过滤
                if (level > level_ 
                    || level > group_->level())
                    return false;
                return true;
            }

        private:
            void log_it(
                size_t level, 
                Record const & record) const;

        private:
            friend class Manager;
            friend class Group;
            friend class Context;

            Module * next_;              /// 下一个模块
            char const * name_;          /// 当前模块的名称
            char const * short_name_;    /// 当前模块的名称
            size_t name_size_;
            GroupBase * group_;              /// 引用的日志模块
            size_t level_;               /// 模块等级，用于一级过滤
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_MODULE_H_
