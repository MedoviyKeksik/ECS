#ifndef ECS_IENTITY_H
#define ECS_IENTITY_H

#include "api.h"
#include "handle.h"

namespace ecs
{
using EntityTypeId                      = TypeID;
using EntityId                          = util::Handle64;
static const EntityId INVALID_ENTITY_ID = util::Handle64::INVALID_HANDLE;

class ECS_API IEntity
{
    friend class EntityManager;

public:
    IEntity();
    virtual ~IEntity();

    template <class T>
    T* GetComponent() const;

    template <class T, class... P>
    T* AddComponent(P&&... param);

    template <class T>
    void RemoveComponent();

    virtual void OnEnable() {}
    virtual void OnDisable() {}

    inline bool operator==(const IEntity& rhs) const
    {
        return this->entityId == rhs.entityId;
    }
    inline bool operator!=(const IEntity& rhs) const
    {
        return this->entityId != rhs.entityId;
    }
    inline bool operator==(const IEntity* rhs) const
    {
        return this->entityId == rhs->entityId;
    }
    inline bool operator!=(const IEntity* rhs) const
    {
        return this->entityId != rhs->entityId;
    }

    virtual const EntityTypeId GetStaticEntityTypeID() const = 0;

    inline const EntityId GetEntityID() const { return this->entityId; }

    void SetActive(bool active);

    inline bool IsActive() const { return this->isActive; }

private:
    // set on create; in EntityManager
    ComponentManager* m_ComponentManagerInstance;

protected:
    DECLARE_STATIC_LOGGER
    EntityId entityId; // set on create; in EntityManager
    bool isActive;   // if false, entity won't be updated
};

} // namespace ecs

#endif // ECS_IENTITY_H
