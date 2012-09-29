// Manager.h

#ifndef _FRAMEWORK_LOGGER_MANAGER_H_
#define _FRAMEWORK_LOGGER_MANAGER_H_

#include "framework/logger/Module.h"
#include "framework/logger/Time.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

namespace framework
{
    namespace configure
    {
        class Config;
    }

    namespace logger
    {

        class Group;
        class Stream;
        class Context;

        /// 日志管理类
        class Manager
        {
        public:
            /// 构造、析构
            Manager();
            ~Manager();

        public:
            /// 从配置文件读取每个输出流，创建输出流队列
            void load_config( 
                framework::configure::Config & conf );

        public:
            /// 注册模块
            Module & register_module(
                char const * name, 
                size_t level);

        public:
            ///.添加一个流
            bool add_stream(
                Stream & s);

            ///.添加一个流到指定日志
            bool add_stream(
                Stream & ls, 
                std::string const & group);

            /// 删除一个流
            bool del_stream(
                Stream & s);

            /// 获取进程线程定义
            Context & context();

        private:
            /// 初始化输出流链表
            void init_streams(
                framework::configure::Config & conf);

            /// 初始化日志链表
            void init_groups(
                framework::configure::Config & conf);

        private:
            framework::configure::Config * config_;

            Stream * streams_;     /// 输出流链表
            Group * groups_;       /// 日志链表
            Group * group_null_;   /// 找不到指定名字的Group时，用这个，不会输出日志
            Module * modules_;     /// 模块链表

            Time time_;
            boost::mutex mutex_;      /// 线程锁
            boost::thread_specific_ptr<Context> thread_ctx_;

            // config
            std::string id_format_;
            std::string defalut_group_;
            size_t defalut_level_;

        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_MANAGER_H_
