#include "memory/allocators/stack_allocator.h"

#include <cassert>

ecs::memory::allocator::StackAllocator::StackAllocator(const std::size_t memorySize, const void* memory)
    : IAllocator(memorySize, memory)
{
}

ecs::memory::allocator::StackAllocator::~StackAllocator()
{
    this->Clear();
}

void* ecs::memory::allocator::StackAllocator::Allocate(const std::size_t size, const u8 alignment)
{
    assert(size > 0 && "allocate called with memorySize = 0.");

    union
    {
        void*              asVoidPointer;
        uptr               asUptr;
        AllocatorMetaInfo* asMetaInfo;
    };

    asVoidPointer = (void*)this->memoryAddress;

    asUptr += this->memoryUsed;

    u8 adjustment = GetAdjustment(asVoidPointer, alignment, sizeof(AllocatorMetaInfo));

    if (this->memoryUsed + size + adjustment > this->memorySize)
    {
        return nullptr;
    }

    asMetaInfo->adjusment = adjustment;

    asUptr += adjustment;

    this->memoryUsed += size + adjustment;
    this->memoryAllocationsCount++;

    return asVoidPointer;
}

void ecs::memory::allocator::StackAllocator::Free(void* memory)
{
    union
    {
        void*              asVoidPointer;
        uptr               asUptr;
        AllocatorMetaInfo* asMetaInfo;
    };

    asVoidPointer = memory;

    asUptr -= sizeof(AllocatorMetaInfo);

    this->memoryUsed -= ((uptr)this->memoryAddress + this->memoryUsed) - ((uptr)memory + asMetaInfo->adjusment);
    this->memoryAllocationsCount--;
}

void ecs::memory::allocator::StackAllocator::Clear()
{
    this->memoryUsed             = 0;
    this->memoryAllocationsCount = 0;
}
