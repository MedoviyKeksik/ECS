#pragma once

//#define ECS_DISABLE_LOGGING 1

#if !ECS_DISABLE_LOGGING
#define DECLARE_LOGGER ecs::log::Logger* LOGGER;
#define DECLARE_STATIC_LOGGER static ecs::log::Logger* LOGGER;

#define DEFINE_LOGGER(name) LOGGER = ecs::log::internal::GetLogger(name);
#define DEFINE_STATIC_LOGGER(clazz, name) log::Logger* clazz::LOGGER = ecs::log::internal::GetLogger(name);
#define DEFINE_STATIC_LOGGER_TEMPLATE(clazz, T, name)                                                                  \
    template <class T>                                                                                                 \
    log::Logger* clazz<T>::LOGGER = ecs::log::internal::GetLogger(name);

#define LogTrace(format, ...) LOGGER->LogTrace(format, ##__VA_ARGS__);
#define LogDebug(format, ...) LOGGER->LogDebug(format, ##__VA_ARGS__);
#define LogInfo(format, ...) LOGGER->LogInfo(format, ##__VA_ARGS__);
#define LogWarning(format, ...) LOGGER->LogWarning(format, ##__VA_ARGS__);
#define LogError(format, ...) LOGGER->LogError(format, ##__VA_ARGS__);
#define LogFatal(format, ...) LOGGER->LogFatal(format, ##__VA_ARGS__);
#else

#define DECLARE_LOGGER
#define DECLARE_STATIC_LOGGER

#define DEFINE_LOGGER(name)
#define DEFINE_STATIC_LOGGER(class, name)
#define DEFINE_STATIC_LOGGER_TEMPLATE(class, T, name)

#define LogTrace(format, ...)
#define LogDebug(format, ...)
#define LogInfo(format, ...)
#define LogWarning(format, ...)
#define LogError(format, ...)
#define LogFatal(format, ...)
#endif
