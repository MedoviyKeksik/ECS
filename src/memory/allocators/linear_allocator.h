#ifndef LINEARALLOCATOR_H
#define LINEARALLOCATOR_H

#include "iallocator.hpp"

namespace ecs
{
namespace memory
{
namespace allocator
{

class LinearAllocator : public IAllocator
{
    /*
     *   Allocates memory in a linear way
     *
     *       first          2    3     4
     *     allocation        allocation
     *         v            v    v     v
     * |=================|=====|===|=======|...|
     * ^                                       ^
     * Initial                                 Last possible
     * memory                                  memory address
     * address                                 (memory + memorySize)
     * (memory)
     */

public:
    LinearAllocator(const std::size_t memorySize, const void* memory);
    virtual ~LinearAllocator();

    virtual void* Allocate(const std::size_t size, const u8 alignment) override;
    virtual void  Free(void* memory) override;
    virtual void  Clear() override;
};

} // namespace allocator
} // namespace Memory
} // namespace ECS

#endif // LINEARALLOCATOR_H
