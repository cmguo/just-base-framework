// Level.h

#ifndef _FRAMEWORK_LOGGER_LEVEL_H_
#define _FRAMEWORK_LOGGER_LEVEL_H_

namespace framework
{
    namespace logger
    {
        /// 日志错误等级定义
        enum LevelEnum
        {
            Off, 
            Fatal, 
            Error, 
            Warn, 
            Info, 
            Debug, 
            Trace,             All, 
            None
        };


    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_LEVEL_H_
