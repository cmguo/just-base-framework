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
            /// 插入一个流
            void add_stream(
                Stream * ls);

            /// 删除一个流
            void del_stream(
                Stream * ls);

        public:
            /// 输出日志
            void log(
                Module const & module, 
                size_t level, 
                Record const & record);

        private:
            struct StreamNode;
            friend class Manager;

            Group * next_;          /// 下一个组
            Manager & manager_;     /// 管理类的引用
            std::string name_;      /// 组名称
            StreamNode * streams_;  /// 拥有的流
      };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_GROUP_H_
