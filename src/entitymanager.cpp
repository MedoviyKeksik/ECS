#include "entitymanager.h"

namespace ecs
{

EntityManager::EntityManager(ComponentManager* componentManagerInstance)
    : pendingDestroyedEntities(1024)
    , numPendingDestroyedEntities(0)
    , componentManager(componentManagerInstance){ DEFINE_LOGGER(
          "EntityManager") LogInfo("InitializeEntityManager!") }

    EntityManager::~EntityManager()
{
    for (auto ec : this->entityRegistry)
    {
        LogDebug("Releasing remaining entities of type '%s' ...",
                 ec.second->GetEntityContainerTypeName());
        delete ec.second;
        ec.second = nullptr;
    }

    LogInfo("Release EntityManager!")
}

EntityId EntityManager::AqcuireEntityId(IEntity* entity)
{
    return this->entityHandleTable.AqcuireHandle(entity);
}

void EntityManager::ReleaseEntityId(EntityId id)
{
    this->entityHandleTable.ReleaseHandle(id);
}

void EntityManager::RemoveDestroyedEntities()
{
    for (size_t i = 0; i < this->numPendingDestroyedEntities; ++i)
    {
        EntityId entityId = this->pendingDestroyedEntities[i];

        IEntity* entity = this->entityHandleTable[entityId];

        const EntityTypeId ETID = entity->GetStaticEntityTypeID();

        // get appropriate entity container and destroy entity
        auto it = this->entityRegistry.find(ETID);
        if (it != this->entityRegistry.end())
        {
            // release entity's components
            this->componentManager->RemoveAllComponents(entityId);

            it->second->DestroyEntity(entity);
        }

        // free entity id
        this->ReleaseEntityId(entityId);
    }

    this->numPendingDestroyedEntities = 0;
}

template <class T, class... ARGS>
EntityId EntityManager::CreateEntity(ARGS&&... args)
{
    // aqcuire memory for new entity object of type T
    void* pObjectMemory = GetEntityContainer<T>()->CreateObject();

    ecs::EntityId entityId = this->AqcuireEntityId((T*)pObjectMemory);

    ((T*)pObjectMemory)->entityId                   = entityId;
    ((T*)pObjectMemory)->m_ComponentManagerInstance = this->componentManager;

    // create entity inplace
    IEntity* entity = new (pObjectMemory) T(std::forward<ARGS>(args)...);

    return entityId;
}
void EntityManager::DestroyEntity(EntityId entityId)
{
    IEntity* entity = this->entityHandleTable[entityId];

    const EntityTypeId ETID = entity->GetStaticEntityTypeID();

    if (this->numPendingDestroyedEntities <
        this->pendingDestroyedEntities.size())
    {
        this->pendingDestroyedEntities[this->numPendingDestroyedEntities++] =
            entityId;
    }
    else
    {
        this->pendingDestroyedEntities.push_back(entityId);
        this->numPendingDestroyedEntities++;
    }
}

IEntity* EntityManager::GetEntity(EntityId entityId)
{
    return this->entityHandleTable[entityId];
}
EntityId EntityManager::GetEntityId(EntityId::value_type index) const
{
    return this->entityHandleTable[index];
}

template <class T>
EntityManager::EntityContainer<T>* EntityManager::GetEntityContainer()
{
    EntityTypeId EID = T::STATIC_ENTITY_TYPE_ID;

    auto                it = this->entityRegistry.find(EID);
    EntityContainer<T>* ec = nullptr;

    if (it == this->entityRegistry.end())
    {
        ec                        = new EntityContainer<T>();
        this->entityRegistry[EID] = ec;
    }
    else
        ec = (EntityContainer<T>*)it->second;

    assert(ec != nullptr && "Failed to create EntityContainer<T>!");
    return ec;
}

} // namespace ecs