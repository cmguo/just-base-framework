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

        /// ��־������
        class Manager
        {
        public:
            /// ���졢����
            Manager();
            ~Manager();

        public:
            /// �������ļ���ȡÿ����������������������
            void load_config( 
                framework::configure::Config & conf );

        public:
            /// ע��ģ��
            Module & register_module(
                char const * name, 
                size_t level);

        public:
            ///.���һ����
            bool add_stream(
                Stream & s);

            ///.���һ������ָ����־
            bool add_stream(
                Stream & ls, 
                std::string const & group);

            /// ɾ��һ����
            bool del_stream(
                Stream & s);

            /// ��ȡ�����̶߳���
            Context & context();

        private:
            /// ��ʼ�����������
            void init_streams(
                framework::configure::Config & conf);

            /// ��ʼ����־����
            void init_groups(
                framework::configure::Config & conf);

        private:
            framework::configure::Config * config_;

            Stream * streams_;     /// ���������
            Group * groups_;       /// ��־����
            Group * group_null_;   /// �Ҳ���ָ�����ֵ�Groupʱ������������������־
            Module * modules_;     /// ģ������

            Time time_;
            boost::mutex mutex_;      /// �߳���
            boost::thread_specific_ptr<Context> thread_ctx_;

            // config
            std::string id_format_;
            std::string defalut_group_;
            size_t defalut_level_;

        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_MANAGER_H_
