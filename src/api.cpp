#include "api.h"

#include "engine.h"
#include "loggermanager.h"
#include "memorymanager.h"

namespace ecs
{
namespace log
{
namespace internal
{

#if !ECS_DISABLE_LOGGING
LoggerManager* EcsLoggerManager = new LoggerManager();
log::Logger*   GetLogger(const char* logger)
{
    return ECSLoggerManager->GetLogger(logger);
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

GlobalMemoryUser::~GlobalMemoryUser() {}

inline const void* GlobalMemoryUser::Allocate(size_t memSize, const char* user)
{
    return internal::ecsMemoryManager->Allocate(memSize, user);
}

inline void GlobalMemoryUser::Free(void* pMem)
{
    internal::ecsMemoryManager->Free(pMem);
}

} // namespace memory

EcsEngine* Ecs_Engine = nullptr; // new ECSEngine();

void Initialize()
{
    if (Ecs_Engine == nullptr)
        Ecs_Engine = new EcsEngine();
}

void Terminate()
{
    if (Ecs_Engine != nullptr)
    {
        delete Ecs_Engine;
        Ecs_Engine = nullptr;
    }

    // check for memory leaks
    memory::internal::ecsMemoryManager->CheckMemoryLeaks();
}
} // namespace ecs