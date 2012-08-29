// Macro.h

#ifndef _FRAMEWORK_LOGGER_MACRO_H_
#define _FRAMEWORK_LOGGER_MACRO_H_

#ifdef LOG_FATAL
#  undef LOG_FATAL
#endif
#define LOG_FATAL(param) LOG(framework::logger::Fatal, param)

#ifdef LOG_ERROR
#  undef LOG_ERROR
#endif
#define LOG_ERROR(param) LOG(framework::logger::Error, param)

#ifdef LOG_WARN
#  undef LOG_WARN
#endif
#define LOG_WARN(param) LOG(framework::logger::Warn, param)

#ifdef LOG_INFO
#  undef LOG_INFO
#endif
#define LOG_INFO(param) LOG(framework::logger::Info, param)

#ifdef LOG_DEBUG
#  undef LOG_DEBUG
#endif
#define LOG_DEBUG(param) LOG(framework::logger::Debug, param)

#ifdef LOG_TRACE
#  undef LOG_TRACE
#endif
#define LOG_TRACE(param) LOG(framework::logger::Trace, param)

#endif // _FRAMEWORK_LOGGER_MACRO_H_
