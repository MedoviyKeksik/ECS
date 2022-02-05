#pragma once

#define ENITY_LUT_GROW 1024
#define ENITY_T_CHUNK_SIZE 512
#define COMPONENT_LUT_GROW 1024
#define COMPONENT_T_CHUNK_SIZE 512
#define ECS_EVENT_MEMORY_BUFFER_SIZE 4194304  // 4MB
#define ECS_SYSTEM_MEMORY_BUFFER_SIZE 8388608 // 8MB

#include "log/logger.h"

#include "platform.h"

namespace ecs
{
namespace log
{
namespace internal
{
#if !ECS_DISABLE_LOGGING

class LoggerManager;
extern LoggerManager* ECSLoggerManager;

/**
 * Returns a log4cpp managed logger instance.
 * @param logger - The logger.
 * @return Null if it fails, else the logger.
 */
ECS_API log::Logger* GetLogger(const char* logger);
#endif
} // namespace internal
} // namespace log

namespace memory
{
namespace internal
{
class MemoryManager;
extern MemoryManager* ECSMemoryManager;
} // namespace internal
} // namespace memory

namespace event
{
class EventHandler;
}

class EntityManager;
class SystemManager;
class ComponentManager;

namespace memory
{
/**
 * Any class that wants to use the global memory must derive from this class.
 */
class ECS_API GlobalMemoryUser
{
private:
    internal::MemoryManager* ecsMemoryManager;

public:
    GlobalMemoryUser();
    virtual ~GlobalMemoryUser() = default;

    const void* Allocate(std::size_t memSize, const char* user = nullptr);
    void        Free(void* pMem);
};

} // namespace memory

class EcsEngine;
ECS_API extern EcsEngine* Ecs_Engine;
ECS_API void              Initialize();
ECS_API void              Terminate();

} // namespace ecs
