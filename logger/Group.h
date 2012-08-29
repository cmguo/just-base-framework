// Group.h

#ifndef _FRAMEWORK_LOGGER_GROUP_H_
#define _FRAMEWORK_LOGGER_GROUP_H_

#include "framework/logger/Module.h"
#include "framework/logger/Record.h"
#include "framework/logger/GroupBase.h"

namespace framework
{
    namespace logger
    {

        class Manager;
        class Stream;

        class Group
            : public GroupBase
        {
        public:
            Group(
                Manager & logmgr);

            ~Group();

        public:
            /// ����һ����
            void add_stream(
                Stream * ls);

            /// ɾ��һ����
            void del_stream(
                Stream * ls);

        public:
            /// �����־
            void log(
                Module const & module, 
                size_t level, 
                Record const & record);

        private:
            struct StreamNode;
            friend class Manager;

            Group * next_;          /// ��һ����
            Manager & manager_;     /// �����������
            std::string name_;      /// ������
            StreamNode * streams_;  /// ӵ�е���
      };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_GROUP_H_
