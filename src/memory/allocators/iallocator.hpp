#ifndef IALLOCATOR_HPP
#define IALLOCATOR_HPP

#include "global.hpp"

namespace ecs
{
namespace memory
{
namespace allocator
{
static inline void* AlignForward(void* address, u8 alignment);
static inline u8    GetAdjustment(const void* address, u8 alignment);
static inline u8    GetAdjustment(const void* address, u8 alignment, u8 extra);

class IAllocator
{
public:
    IAllocator(const std::size_t memorySize, const void* memory);
    virtual ~IAllocator();

    virtual void* Allocate(const std::size_t size, const u8 alignment) = 0;
    virtual void  Free(void* memory)                                   = 0;
    virtual void  Clear()                                              = 0;

    void               SetMemorySize(const std::size_t) = delete;
    inline std::size_t GetMemorySize() const { return this->memorySize; }

    void               SetMemoryAddress(const void*) = delete;
    inline const void* GetMemoryAddress() const { return this->memoryAddress; }

    void               SetUsedMemory(const std::size_t) = delete;
    inline std::size_t GetUsedMemory() const { return this->memoryUsed; }

    void       SetAllocationsCount(const u64) = delete;
    inline u64 GetAllocationsCount() const
    {
        return this->memoryAllocationsCount;
    }

protected:
    const std::size_t memorySize;
    const void*       memoryAddress;
    std::size_t       memoryUsed;
    u64               memoryAllocationsCount;
};
} // namespace allocator
} // namespace memory
} // namespace ecs

#endif
