#ifndef ECS_API_H
#define ECS_API_H

#pragma once

#define ENITY_LUT_GROW 1024

#define ENITY_T_CHUNK_SIZE 512

#define COMPONENT_LUT_GROW 1024

#define COMPONENT_T_CHUNK_SIZE 512

// 4MB
#define ECS_EVENT_MEMORY_BUFFER_SIZE 4194304

// 8MB
#define ECS_SYSTEM_MEMORY_BUFFER_SIZE 8388608

#include "platform.h"
#include "log/logger.h"

namespace ecs
{
namespace log::internal
{
#if !ECS_DISABLE_LOGGING

class LoggerManager;
extern LoggerManager* ECSLoggerManager;

log::Logger* GetLogger(const char* logger);
#endif
} // namespace log::internal

namespace memory::internal
{
class MemoryManager;
extern MemoryManager* ECSMemoryManager;
} // namespace memory::internal

namespace event
{
class EventHandler;
}

class EntityManager;
class SystemManager;
class ComponentManager;

namespace memory
{
class GlobalMemoryUser
{
public:
    GlobalMemoryUser();
    virtual ~GlobalMemoryUser();

    inline const void* Allocate(size_t memSize, const char* user = nullptr);
    inline void        Free(void* pMem);

private:
    internal::MemoryManager* ECS_MEMORY_MANAGER;
};

} // namespace memory

class ECSEngine;

extern ECSEngine* ECS_Engine;

void Initialize();
void Terminate();

} // namespace ecs

#endif // ECS_API_H
