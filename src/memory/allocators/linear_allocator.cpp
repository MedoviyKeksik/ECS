#include "memory/allocators/linear_allocator.h"

#include <cassert>

ecs::memory::allocator::LinearAllocator::LinearAllocator(
    const std::size_t memorySize, const void* memory)
    : IAllocator(memorySize, memory)
{
}

ecs::memory::allocator::LinearAllocator::~LinearAllocator()
{
    this->Clear();
}

void* ecs::memory::allocator::LinearAllocator::Allocate(const std::size_t size,
                                                        const u8 alignment)
{
    assert(size > 0 && "allocate calles with memSize = 0.");

    union
    {
        void* asVoidPointer;
        uptr  asUptr;
    };

    asVoidPointer = (void*)this->memoryAddress;
    asUptr += this->memoryUsed;

    u8 adjustment = GetAdjustment(asVoidPointer, alignment);

    if (this->memoryUsed + size + adjustment > this->memorySize)
    {
        return nullptr;
    }

    asUptr += adjustment;

    this->memoryUsed += size + adjustment;
    this->memoryAllocationsCount++;

    return asVoidPointer;
}

void ecs::memory::allocator::LinearAllocator::Free(void* memory)
{
    assert(
        false &&
        "Linear allocators do not support free operations. Use clear unstead.");
}

void ecs::memory::allocator::LinearAllocator::Clear()
{
    this->memoryUsed             = 0;
    this->memoryAllocationsCount = 0;
}
