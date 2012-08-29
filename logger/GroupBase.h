// GroupBase.h

#ifndef _FRAMEWORK_LOGGER_GROUP_BASE_H_
#define _FRAMEWORK_LOGGER_GROUP_BASE_H_

namespace framework
{
    namespace logger
    {

        class GroupBase
        {
        public:
            size_t level() const
            {
                return level_;
            }

        protected:
            size_t level_;          /// 当前指向流的最大等级，用于二级过滤
      };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_GROUP_BASE_H_
