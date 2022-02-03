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

public:
    IComponent();
    virtual ~IComponent();

    inline const bool operator==(const IComponent& other) const
    {
        return hashValue == other.hashValue;
    }
    inline const bool operator!=(const IComponent& other) const
    {
        return hashValue == other.hashValue;
    }

    inline const ComponentId GetComponentId() const
    {
        return this->componentId;
    }

    inline const EntityId GetOwner() const { return this->owner; }

    inline void SetActive(bool state) { this->enabled = state; }
    inline bool IsActive() const { return this->enabled; }

protected:
    ComponentId hashValue;
    ComponentId componentId;
    EntityId    owner;
    bool        enabled;
};
} // namespace ecs

#endif // ECS_ICOMPONENT_H
