#ifndef ECS_ICOMPONENT_H
#define ECS_ICOMPONENT_H

#include "ientity.h"

namespace ecs
{
using ComponentId     = ObjectID;
using ComponentTypeId = TypeID;

static const ComponentId INVALID_COMPONENT_ID = INVALID_OBJECT_ID;

template <class T>
class Component;

class ECS_API IComponent
{
    friend class ComponentManager;

protected:
    ComponentId m_HashValue;

    ComponentId m_ComponentID;

    EntityId m_Owner;

    bool m_Enabled;

public:
    IComponent();

    virtual ~IComponent();

    // COMPARE
    inline const bool operator==(const IComponent& other) const
    {
        return m_HashValue == other.m_HashValue;
    }
    inline const bool operator!=(const IComponent& other) const
    {
        return m_HashValue == other.m_HashValue;
    }

    // ACCESSOR
    inline const ComponentId GetComponentId() const
    {
        return this->m_ComponentID;
    }

    inline const EntityId GetOwner() const { return this->m_Owner; }

    inline void SetActive(bool state) { this->m_Enabled = state; }
    inline bool IsActive() const { return this->m_Enabled; }
};
} // namespace ecs

#endif // ECS_ICOMPONENT_H
