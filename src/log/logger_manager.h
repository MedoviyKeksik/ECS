#pragma once

//#include "logger_macro.h"

#if !ECS_DISABLE_LOGGING

#include "platform.h"

// Log4cplus logger support
#include "log4cplus/logger.h"
#include "logger.h"
#include "log4cplus/initializer.h"

namespace ecs::log
{

// class Logger;

namespace internal
{

class ECS_API LoggerManager
{
    using LoggerCache = std::unordered_map<std::string, Logger*>;

    static constexpr const char* LOG_FILE_NAME  = "ECS.log";
    static constexpr const char* DEFAULT_LOGGER = "ECS";
    static constexpr const char* LOG_PATTERN    = "%d{%H:%M:%S,%q} [%t] %-5p %c{1} %x- %m%n";

    // This class is not inteeded to be initialized

    // root logger
    log4cplus::Logger m_RootLogger;

    /// Summary:	Holds all acquired logger
    LoggerCache m_Cache;

public:
    LoggerManager();
    ~LoggerManager();
    LoggerManager(const LoggerManager&) = delete;
    LoggerManager& operator=(LoggerManager&) = delete;
    Logger*        GetLogger(const char* logger = DEFAULT_LOGGER);
    log4cplus::Initializer initializer;
}; // class LoggerManager

} // namespace internal
} // namespace ecs::log

#endif // !ECS_DISABLE_LOGGING
