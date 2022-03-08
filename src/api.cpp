#include "api.h"

#include "engine.h"
#include "log/logger_manager.h"
#include "memory/memory_manager.h"

namespace ecs
{
namespace log
{
namespace internal
{

#if !ECS_DISABLE_LOGGING
LoggerManager* ecsLoggerManager;
log::Logger*   GetLogger(const char* logger)
{
    if (ecsLoggerManager == NULL)
        ecsLoggerManager = new LoggerManager();
    return ecsLoggerManager->GetLogger(logger);
}
#endif

} // namespace internal
} // namespace log

namespace memory
{
namespace internal
{

MemoryManager* ecsMemoryManager = new memory::internal::MemoryManager();

} // namespace internal

GlobalMemoryUser::GlobalMemoryUser()
    : ecsMemoryManager(internal::ecsMemoryManager)
{
}

const void* GlobalMemoryUser::Allocate(std::size_t memSize, const char* user)
{
    return ecsMemoryManager->Allocate(memSize, user);
}

void GlobalMemoryUser::Free(void* pMem)
{
    ecsMemoryManager->Free(pMem);
}

} // namespace memory

EcsEngine* ecsEngine = nullptr; // new ECSEngine();

void Initialize()
{
    if (ecsEngine == nullptr)
        ecsEngine = new EcsEngine();
}

void Terminate()
{
    if (ecsEngine != nullptr)
    {
        delete ecsEngine;
        ecsEngine = nullptr;
    }

    // check for memory leaks
    memory::internal::ecsMemoryManager->CheckMemoryLeaks();
}
} // namespace ecs
