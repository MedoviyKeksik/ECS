#pragma once

#ifndef ECS_PLATFORM_H
#define ECS_PLATFORM_H

// Platform includes
#include <limits>
#include <climits>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <algorithm>
#include <functional>

#include <assert.h>
#include <memory>

#include <math.h>
#include <string>

#include <chrono>

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

    explicit TimeStamp(f32 floatValue)
        : asFloat(floatValue)
    {
    }

    explicit operator u32() const { return this->asUInt; }

    inline bool operator==(const TimeStamp& other) const
    {
        return this->asUInt == other.asUInt;
    }
    inline bool operator!=(const TimeStamp& other) const
    {
        return this->asUInt != other.asUInt;
    }

    inline bool operator<(const TimeStamp& other) const
    {
        return this->asFloat < other.asFloat;
    }
    inline bool operator>(const TimeStamp& other) const
    {
        return this->asFloat > other.asFloat;
    }

}; // union TimeStamp

} // namespace ecs
#endif // ECS_PLATFORM_H
