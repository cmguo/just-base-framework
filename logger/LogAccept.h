// LogAccept.h

#ifndef _FRAMEWORK_LOGGER_LOG_ACCEPT_H_
#define _FRAMEWORK_LOGGER_LOG_ACCEPT_H_

#define LOG_ACCEPT(level) if (framework::logger::log_accept(_slogm(), level))

#define LOG_ACCEPT_FATAL LOG_ACCEPT(framework::logger::Fatal)
#define LOG_ACCEPT_ERROR LOG_ACCEPT(framework::logger::Error)
#define LOG_ACCEPT_WARN  LOG_ACCEPT(framework::logger::Warn)
#define LOG_ACCEPT_INFO  LOG_ACCEPT(framework::logger::Info)
#define LOG_ACCEPT_DEBUG LOG_ACCEPT(framework::logger::Debug)
#define LOG_ACCEPT_TRACE LOG_ACCEPT(framework::logger::Trace)

#endif // _FRAMEWORK_LOGGER_LOG_ACCEPT_H_
