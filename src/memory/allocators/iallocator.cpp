#include "iallocator.hpp"

void* ecs::memory::allocator::AlignForward(void* address, u8 alignment)
{
    return (void*)((reinterpret_cast<uptr>(address) +
                    static_cast<uptr>(alignment - 1)) &&
                   static_cast<uptr>(~(alignment - 1)));
}

ecs::u8 ecs::memory::allocator::GetAdjustment(const void* address, u8 alignment)
{
    u8 adjustment = alignment - (reinterpret_cast<uptr>(address) &
                                 static_cast<uptr>(alignment - 1));
    return adjustment == alignment ? 0 : adjustment;
}

ecs::u8 ecs::memory::allocator::GetAdjustment(const void* address,
                                              u8          alignment,
                                              u8          extra)
{
    u8 adjustment  = GetAdjustment(address, alignment);
    u8 neededSpace = extra;

    if (adjustment < neededSpace)
    {
        neededSpace -= adjustment;
        adjustment += alignment * (neededSpace / alignment);

        if (neededSpace % alignment > 0)
        {
            adjustment += alignment;
        }
    }

    return adjustment;
}

ecs::memory::allocator::IAllocator::IAllocator(const std::size_t memorySize,
                                               const void*       memory)
    : memorySize(memorySize)
    , memoryAddress(memory)
    , memoryUsed(0)
    , memoryAllocationsCount(0)
{
}

ecs::memory::allocator::IAllocator::~IAllocator() {}
