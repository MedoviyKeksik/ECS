#pragma once

#include "api.h"
#include "memory/allocators/stack_allocator.h"
#include "log/logger_macro.h"

#define ECS_GLOBAL_MEMORY_CAPACITY 134217728 // 128 MB

namespace ecs
{
namespace memory
{
namespace internal
{

class ECS_API MemoryManager
{
    using StackAllocator = allocator::StackAllocator;

    DECLARE_LOGGER

public:
    MemoryManager();
    ~MemoryManager();

    void* Allocate(std::size_t memorySize, const std::string& user = "");

    void Free(void* pointerMemory);

    void CheckMemoryLeaks();

public:
    static constexpr std::size_t MEMORY_CAPACITY = ECS_GLOBAL_MEMORY_CAPACITY;

private:
    void*                                            globalMemory;
    StackAllocator*                                  memoryAllocator;
    std::vector<std::pair<const std::string, void*>> pendingMemory;
    std::list<void*>                                 freedMemory;

    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(MemoryManager&) = delete;
};

} // namespace internal
} // namespace memory
} // namespace ecs
