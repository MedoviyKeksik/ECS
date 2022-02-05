#include "pool_allocator.h"

#include <cassert>
#include <cmath>

ecs::memory::allocator::PoolAllocator::PoolAllocator(
    const std::size_t memorySize,
    const void*       memory,
    const std::size_t objectSize,
    const u8          objectAlignment)
    : IAllocator(memorySize, memory)
    , objectSize(objectSize)
    , objectAlignment(objectAlignment)
{
    this->Clear();
}

ecs::memory::allocator::PoolAllocator::~PoolAllocator()
{
    this->freeList = nullptr;
}

void* ecs::memory::allocator::PoolAllocator::Allocate(const std::size_t size,
                                                      const u8 alignment)
{
    assert(size > 0 && "allocate called with memorySize = 0.");
    assert(size == this->objectSize && alignment == this->objectAlignment);

    if (this->freeList == nullptr)
    {
        return nullptr;
    }

    void* pointer = this->freeList;

    this->freeList = (void**)(*this->freeList);

    this->memoryUsed += this->objectSize;
    this->memoryAllocationsCount++;

    return pointer;
}

void ecs::memory::allocator::PoolAllocator::Free(void* memory)
{
    *((void**)memory) = this->freeList;
    this->freeList    = (void**)memory;

    this->memoryUsed -= this->objectSize;
    this->memoryAllocationsCount--;
}

void ecs::memory::allocator::PoolAllocator::Clear()
{
    u8 adjustment = GetAdjustment(this->memoryAddress, this->objectAlignment);
    std::size_t numberObjects =
        (std::size_t)floor((this->memorySize - adjustment) / this->objectSize);

    union
    {
        void* asVoidPointer;
        uptr  asUptd;
    };

    asVoidPointer = (void*)this->memoryAddress;

    asUptd += adjustment;

    this->freeList = (void**)asVoidPointer;

    void** pointer = this->freeList;

    for (int i = 0; i < (numberObjects - 1); ++i)
    {
        *pointer = (void*)((uptr)pointer + this->objectSize);
        pointer  = (void**)*pointer;
    }

    *pointer = nullptr;
}
