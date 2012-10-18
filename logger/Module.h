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

        /// ģ�鶨��
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
                // ���̺߳�����̬������ʼ���г�ͻ������Ϊ��ָ��
                if (this == NULL)
                    return;
                /// ���������ȼ�����
                if (level > level_ 
                    || level > group_->level())
                    return;
                log_it(level, record);

            }

            bool log_accept(
                size_t level) const
            {
                // ���̺߳�����̬������ʼ���г�ͻ������Ϊ��ָ��
                if (this == NULL)
                    return false;
                /// ���������ȼ�����
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

            Module * next_;              /// ��һ��ģ��
            char const * name_;          /// ��ǰģ�������
            char const * short_name_;    /// ��ǰģ�������
            size_t name_size_;
            GroupBase * group_;              /// ���õ���־ģ��
            size_t level_;               /// ģ��ȼ�������һ������
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_MODULE_H_
