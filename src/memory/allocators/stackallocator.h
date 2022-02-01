#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

#include "iallocator.hpp"

namespace ecs
{
namespace memory
{
namespace allocator
{

class StackAllocator : public IAllocator
{
public:
    StackAllocator(const std::size_t memorySize, const void* memory);
    virtual ~StackAllocator();

    virtual void* Allocate(const std::size_t size, const u8 alignment) override;
    virtual void  Free(void* memory) override;
    virtual void  Clear() override;

private:
    struct AllocatorMetaInfo
    {
        u8 adjusment;
    };
};

} // namespace allocator
} // namespace Memory
} // namespace ECS

#endif // STACKALLOCATOR_H
