#pragma once

#include "platform.h"

#include "logger_macro.h"

#if !ECS_DISABLE_LOGGING
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"

namespace ecs::log
{

class ECS_API Logger
{
    log4cplus::Logger m_logger;

public:
    Logger(const Logger&) = delete;
    Logger& operator=(Logger&) = delete;

    explicit Logger(log4cplus::Logger& logger) { m_logger = logger; };

    ~Logger() = default;

    // trace
    template <typename... Args>
    inline void LogTrace(const char* fmt, Args... args)
    {
        LOG4CPLUS_TRACE_FMT(this->m_logger,
                            (const log4cplus::tchar*)fmt,
                            std::forward<Args>(args)...);
    }

    // debug
    template <typename... Args>
    inline void LogDebug(const char* fmt, Args... args)
    {
        LOG4CPLUS_DEBUG_FMT(this->m_logger, fmt, std::forward<Args>(args)...);
    }

    // info
    template <typename... Args>
    inline void LogInfo(const char* fmt, Args... args)
    {
        LOG4CPLUS_INFO_FMT(this->m_logger, fmt, std::forward<Args>(args)...);
    }

    // warn
    template <typename... Args>
    inline void LogWarning(const char* fmt, Args... args)
    {
        LOG4CPLUS_WARN_FMT(this->m_logger, fmt, std::forward<Args>(args)...);
    }

    // error
    template <typename... Args>
    inline void LogError(const char* fmt, Args... args)
    {
        LOG4CPLUS_ERROR_FMT(this->m_logger, fmt, std::forward<Args>(args)...);
    }

    // fatal
    template <typename... Args>
    inline void LogFatal(const char* fmt, Args... args)
    {
        LOG4CPLUS_FATAL_FMT(this->m_logger, fmt, std::forward<Args>(args)...);
    }

}; // class Logger

} // namespace ecs::log

#endif // !ECS_DISABLE_LOGGING
