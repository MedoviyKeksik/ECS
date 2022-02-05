#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H

#include "iallocator.hpp"

namespace ecs
{
namespace memory
{
namespace allocator
{

class ECS_API PoolAllocator : public IAllocator
{
public:
    PoolAllocator(const std::size_t memorySize,
                  const void*       memory,
                  const std::size_t objectSize,
                  const u8          objectAlignment);
    virtual ~PoolAllocator();

    virtual void* Allocate(const std::size_t size, const u8 alignment) override;
    virtual void  Free(void* memory) override;
    virtual void  Clear() override;

private:
    const std::size_t objectSize;
    const u8          objectAlignment;
    void**            freeList;
};

} // namespace allocator
} // namespace memory
} // namespace ecs

#endif // POOLALLOCATOR_H
