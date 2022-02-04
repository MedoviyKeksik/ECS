#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <list>
#include <string>
#include <vector>

#include "global.hpp"
#include "stackallocator.h"

#define ECS_GLOBAL_MEMORY_CAPACITY 134217728 // 128 MB

namespace ecs
{
namespace memory
{
namespace Internal
{

class MemoryManager
{

    using StackAllocator = allocator::StackAllocator;

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

} // namespace Internal
} // namespace Memory
} // namespace ECS

#endif // MEMORYMANAGER_H
