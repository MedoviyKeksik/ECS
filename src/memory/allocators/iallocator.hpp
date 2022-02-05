#pragma once

#include "api.h"

namespace ecs
{
namespace memory
{
namespace allocator
{
static inline void* AlignForward(void* address, u8 alignment)
{
    return (void*)((reinterpret_cast<uptr>(address) +
                    static_cast<uptr>(alignment - 1)) &&
                   static_cast<uptr>(~(alignment - 1)));
}
static inline u8 GetAdjustment(const void* address, u8 alignment)
{
    u8 adjustment = alignment - (reinterpret_cast<uptr>(address) &
                                 static_cast<uptr>(alignment - 1));
    return adjustment == alignment ? 0 : adjustment;
}
static inline u8 GetAdjustment(const void* address, u8 alignment, u8 extra)
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

class ECS_API IAllocator
{
public:
    IAllocator(const std::size_t memorySize, const void* memory);
    virtual ~IAllocator() = default;

    virtual void* Allocate(const std::size_t size, const u8 alignment) = 0;
    virtual void  Free(void* memory)                                   = 0;
    virtual void  Clear()                                              = 0;

    inline void       SetMemorySize(const std::size_t) = delete;
    inline const auto GetMemorySize() const { return this->memorySize; }
    inline auto       GetMemorySize() { return this->memorySize; }

    inline void       SetMemoryAddress(const void*) = delete;
    inline const auto GetMemoryAddress() const { return this->memoryAddress; }
    inline auto       GetMemoryAddress() { return this->memoryAddress; }

    inline void       SetUsedMemory(const std::size_t) = delete;
    inline const auto GetUsedMemory() const { return this->memoryUsed; }
    inline auto       GetUsedMemory() { return this->memoryUsed; }

    inline void       SetAllocationsCount(const u64) = delete;
    inline const auto GetAllocationsCount() const
    {
        return this->memoryAllocationsCount;
    }
    inline auto GetAllocationsCount() { return this->memoryAllocationsCount; }

protected:
    const std::size_t memorySize;
    const void*       memoryAddress;
    std::size_t       memoryUsed;
    u64               memoryAllocationsCount;
};
} // namespace allocator
} // namespace memory
} // namespace ecs
