#ifndef ECS_ENTITYMANAGER_H
#define ECS_ENTITYMANAGER_H

#include "api.h"
#include "engine.h"
#include "ientity.h"
#include "componentmanager.h
#include "handle.h"
#include "memorychunkallocator.h"

namespace ecs
{
using EntityHandleTable = util::HandleTable<IEntity, EntityId>;

class ECS_API EntityManager
{
    DECLARE_LOGGER

    class IEntityContainer
    {
    public:
        virtual ~IEntityContainer() {}

        virtual const char* GetEntityContainerTypeName() const = 0;

        virtual void DestroyEntity(IEntity* object) = 0;

    }; // class IEntityContainer

    /**
     * An entity container that manages memory chunks of enities T.
     * @tparam T - Generic type parameter.
     */
    template <class T>
    class EntityContainer
        : public memory::MemoryChunkAllocator<T, ENITY_T_CHUNK_SIZE>,
          public IEntityContainer
    {

        EntityContainer(const EntityContainer&) = delete;
        EntityContainer& operator=(EntityContainer&) = delete;

    public:
        EntityContainer()
            : MemoryChunkAllocator("EntityManager")
        {
        }

        virtual ~EntityContainer() {}

        virtual const char* GetEntityContainerTypeName() const override
        {
            static const char* ENTITY_TYPE_NAME{ typeid(T).name() };
            return ENTITY_TYPE_NAME;
        }

        virtual void DestroyEntity(IEntity* object) override
        {
            // call d'tor
            object->~IEntity();

            this->DestroyObject(object);
        }

    }; // EntityContainer

    using EntityRegistry = std::unordered_map<EntityTypeId, IEntityContainer*>;
    EntityRegistry m_EntityRegistry;

    using PendingDestroyedEntities = std::vector<EntityId>;
    PendingDestroyedEntities m_PendingDestroyedEntities;
    size_t                   m_NumPendingDestroyedEntities;

    ComponentManager* m_ComponentManagerInstance;

private:
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(EntityManager&) = delete;

    EntityHandleTable m_EntityHandleTable;

    /**
     * Returns/Creates an entity container for entities of type T.
     * @tparam T - Generic type parameter.
     * @return Null if it fails, else the entity container.
     */
    template <class T>
    inline EntityContainer<T>* GetEntityContainer()
    {
        EntityTypeId EID = T::STATIC_ENTITY_TYPE_ID;

        auto                it = this->m_EntityRegistry.find(EID);
        EntityContainer<T>* ec = nullptr;

        if (it == this->m_EntityRegistry.end())
        {
            ec                          = new EntityContainer<T>();
            this->m_EntityRegistry[EID] = ec;
        }
        else
            ec = (EntityContainer<T>*)it->second;

        assert(ec != nullptr && "Failed to create EntityContainer<T>!");
        return ec;
    }

    /**
     * Aqcuire entity identifier. This method will be used by IEntity class
     * c'tor to set the entity id on creation.
     * @param entity - [in,out] If non-null, the entity.
     * @return An EntityId.
     */
    EntityId AqcuireEntityId(IEntity* entity);

    /**
     * Releases the entity identifier for reuse.
     * @param id - The identifier.
     */
    void ReleaseEntityId(EntityId id);

public:
    EntityManager(ComponentManager* componentManagerInstance);
    ~EntityManager();

    /**
     * Creates an entity of type T and returns its id.
     * @tparam T - Generic type parameter.
     * @tparam ARGS - Type of the arguments.
     * @param args - Variable arguments providing [in,out] The arguments.
     * @return The new entity.
     */
    template <class T, class... ARGS>
    EntityId CreateEntity(ARGS&&... args)
    {
        // aqcuire memory for new entity object of type T
        void* pObjectMemory = GetEntityContainer<T>()->CreateObject();

        ecs::EntityId entityId = this->AqcuireEntityId((T*)pObjectMemory);

        ((T*)pObjectMemory)->m_EntityID = entityId;
        ((T*)pObjectMemory)->m_ComponentManagerInstance =
            this->m_ComponentManagerInstance;

        // create entity inplace
        IEntity* entity = new (pObjectMemory) T(std::forward<ARGS>(args)...);

        return entityId;
    }

    void DestroyEntity(EntityId entityId)
    {
        IEntity* entity = this->m_EntityHandleTable[entityId];

        const EntityTypeId ETID = entity->GetStaticEntityTypeID();

        if (this->m_NumPendingDestroyedEntities <
            this->m_PendingDestroyedEntities.size())
        {
            this->m_PendingDestroyedEntities
                [this->m_NumPendingDestroyedEntities++] = entityId;
        }
        else
        {
            this->m_PendingDestroyedEntities.push_back(entityId);
            this->m_NumPendingDestroyedEntities++;
        }
    }

    /**
     * Get an entity object by its id.
     * @param entityId - The identifier.
     * @return Null if it fails, else the entity.
     */
    inline IEntity* GetEntity(EntityId entityId)
    {
        return this->m_EntityHandleTable[entityId];
    }

    /**
     * Gets the currently stored entity identifier at the given index.
     * @param index - Zero-based index of the.
     * @return The entity identifier.
     */
    inline EntityId GetEntityId(EntityId::value_type index) const
    {
        return this->m_EntityHandleTable[index];
    }

    /**
     * Removes all destroyed entities.
     */
    void RemoveDestroyedEntities();
};

} // namespace ecs

#endif // ECS_ENTITYMANAGER_H
