#ifndef ECS_ENTITY_H
#define ECS_ENTITY_H

#include "ientity.h"

namespace ecs
{

/**
 * CRTP class. Any entity object should derive form the Entity class and passes
 * itself as template parameter to the Entity class.
 * @tparam E - Type of the e.
 */
template <class E>
class Entity : public IEntity
{
    // Entity destruction always happens through EntityManager !!!
    void operator delete(void*)   = delete;
    void operator delete[](void*) = delete;

public:
    static const EntityTypeId STATIC_ENTITY_TYPE_ID;

public:
    virtual const EntityTypeId GetStaticEntityTypeID() const override
    {
        return STATIC_ENTITY_TYPE_ID;
    }

    Entity() {}

    virtual ~Entity() {}
};

// set unique type id for this Entity<T>
template <class E>
const EntityTypeId Entity<E>::STATIC_ENTITY_TYPE_ID =
    util::internal::FamilyTypeID<IEntity>::Get<E>();
} // namespace ecs
#endif // ECS_ENTITY_H
