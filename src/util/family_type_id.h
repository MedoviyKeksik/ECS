#pragma once

#include "api.h"

namespace ecs::util::internal
{

template <class T>
class ECS_API FamilyTypeID
{
public:
    template <class U>
    static const TypeID Get()
    {
        static const TypeID STATIC_TYPE_ID{ s_count++ };
        return STATIC_TYPE_ID;
    }

    static TypeID Get() { return s_count; }

private:
    static TypeID s_count;
};

} // namespace ecs::util::internal
