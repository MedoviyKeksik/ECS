#include "iallocator.hpp"

ecs::memory::allocator::IAllocator::IAllocator(const std::size_t memorySize,
                                               const void*       memory)
    : memorySize(memorySize)
    , memoryAddress(memory)
    , memoryUsed(0)
    , memoryAllocationsCount(0)
{
}
