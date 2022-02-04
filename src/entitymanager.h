#ifndef ECS_ENTITYMANAGER_H
#define ECS_ENTITYMANAGER_H

#include "api.h"
#include "componentmanager.h"
#include "engine.h"
#include "handle.h"
#include "ientity.h"
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
    public:
        EntityContainer()
            : MemoryChunkAllocator("EntityManager")
        {
        }
        virtual ~EntityContainer() {}

    private:
        EntityContainer(const EntityContainer&) = delete;
        EntityContainer& operator=(EntityContainer&) = delete;

    public:
        virtual const char* GetEntityContainerTypeName() const override
        {
            static const char* ENTITY_TYPE_NAME{ typeid(T).name() };
            return ENTITY_TYPE_NAME;
        }

        virtual void DestroyEntity(IEntity* object) override
        {
            object->~IEntity();
            this->DestroyObject(object);
        }

    }; // EntityContainer

public:
    EntityManager(ComponentManager* componentManagerInstance);
    ~EntityManager();

private:
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(EntityManager&) = delete;

public:
    /**
     * Creates an entity of type T and returns its id.
     * @tparam T - Generic type parameter.
     * @tparam ARGS - Type of the arguments.
     * @param args - Variable arguments providing [in,out] The arguments.
     * @return The new entity.
     */
    template <class T, class... ARGS>
    EntityId CreateEntity(ARGS&&... args);

    void DestroyEntity(EntityId entityId);

    /**
     * Get an entity object by its id.
     * @param entityId - The identifier.
     * @return Null if it fails, else the entity.
     */
    inline IEntity* GetEntity(EntityId entityId);

    /**
     * Gets the currently stored entity identifier at the given index.
     * @param index - Zero-based index of the.
     * @return The entity identifier.
     */
    inline EntityId GetEntityId(EntityId::value_type index) const;

    /**
     * Removes all destroyed entities.
     */
    void RemoveDestroyedEntities();

private:
    /**
     * Returns/Creates an entity container for entities of type T.
     * @tparam T - Generic type parameter.
     * @return Null if it fails, else the entity container.
     */
    template <class T>
    inline EntityContainer<T>* GetEntityContainer();

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

private:
    using EntityRegistry = std::unordered_map<EntityTypeId, IEntityContainer*>;
    using PendingDestroyedEntities = std::vector<EntityId>;
    EntityRegistry           entityRegistry;
    PendingDestroyedEntities pendingDestroyedEntities;
    size_t                   numPendingDestroyedEntities;
    ComponentManager*        componentManager;
    EntityHandleTable        entityHandleTable;
};

} // namespace ecs

#endif // ECS_ENTITYMANAGER_H
