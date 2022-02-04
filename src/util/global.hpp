#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <cstdint>
#include <limits>

namespace ecs
{
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using ObjectID = std::size_t;
using TypeID   = std::size_t;

using iptr = intptr_t;
using uptr = uintptr_t;

static const ObjectID INVALID_OBJECT_ID = std::numeric_limits<ObjectID>::max();
static const TypeID   INVALID_TYPE_ID   = std::numeric_limits<TypeID>::max();

union TimeStamp
{
    f32 asFloat;
    u32 asUInt;

    TimeStamp()
        : asUInt(0U)
    {
    }

    TimeStamp(f32 floatValue)
        : asFloat(floatValue)
    {
    }

    operator u32() const { return this->asUInt; }

    inline const bool operator==(const TimeStamp& other) const
    {
        return this->asUInt == other.asUInt;
    }
    inline const bool operator!=(const TimeStamp& other) const
    {
        return this->asUInt != other.asUInt;
    }

    inline const bool operator<(const TimeStamp& other) const
    {
        return this->asFloat < other.asFloat;
    }
    inline const bool operator>(const TimeStamp& other) const
    {
        return this->asFloat > other.asFloat;
    }

}; // union TimeStamp

namespace memory
{
namespace Internal
{
class MemoryManager;
}

class GlobalMemoryUser
{
private:
    Internal::MemoryManager* ecsMemoryManager;

public:
    GlobalMemoryUser();
    virtual ~GlobalMemoryUser();

    inline const void* Allocate(std::size_t memSize,
                                const char* user = nullptr);
    inline void        Free(void* pMem);
};

} // namespace memory

} // namespace ecs

#endif
