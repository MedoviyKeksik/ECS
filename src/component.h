#ifndef ECS_COMPONENT_H
#define ECS_COMPONENT_H

#include "api.h"
#include "familytypeid.h"
#include "icomponent.h"

namespace ecs
{
template <class T>
class Component : public IComponent
{
public:
    static const ComponentTypeId STATIC_COMPONENT_TYPE_ID;

    Component() {}

    virtual ~Component() {}

    inline ComponentTypeId GetStaticComponentTypeID() const
    {
        return STATIC_COMPONENT_TYPE_ID;
    }
};

// This private member only exists to force the compiler to create an instance
// of Component T, which will set its unique identifier.
template <class T>
const ComponentTypeId Component<T>::STATIC_COMPONENT_TYPE_ID =
    util::Internal::FamilyTypeID<IComponent>::Get<T>();
} // namespace ecs

#endif // ECS_COMPONENT_H
