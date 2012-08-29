// Module.cpp

#include "framework/Framework.h"
#include "framework/logger/Module.h"
#include "framework/logger/Group.h"

namespace framework
{
    namespace logger
    {

        Module::Module(
            GroupBase * group, 
            char const * name, 
            size_t level)
            : next_(NULL)
            , group_(group)
            , name_(name)
            , level_(level)
        {
            short_name_ = strrchr(name, '.');
            short_name_ = short_name_ ? short_name_ + 1 : name;
            name_size_ = strlen(short_name_);
        }

        void Module::log_it(
            size_t level, 
            Record const & record) const
        {
            static_cast<Group *>(group_)->log(*this, level, record);
        }


    } // namespace logger
} // namespace framework
