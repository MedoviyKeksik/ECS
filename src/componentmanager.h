#ifndef ECS_COMPONENTMANAGER_H
#define ECS_COMPONENTMANAGER_H

#include "api.h"
#include "engine.h"
#include "familytypeid.h"
#include "icomponent.h"
#include "memorychunkallocator.h"

namespace ecs
{
class ECS_API ComponentManager : memory::GlobalMemoryUser
{
    friend class IComponent;
    DECLARE_LOGGER

    class IComponentContainer
    {
    public:
        virtual ~IComponentContainer() {}

        virtual const char* GetComponentContainerTypeName() const = 0;

        virtual void DestroyComponent(IComponent* object) = 0;
    };

    template <class T>
    class ComponentContainer
        : public memory::MemoryChunkAllocator<T, COMPONENT_T_CHUNK_SIZE>,
          public IComponentContainer
    {
        ComponentContainer(const ComponentContainer&) = delete;
        ComponentContainer& operator=(ComponentContainer&) = delete;

    public:
        ComponentContainer()
            : MemoryChunkAllocator("ComponentManager")
        {
        }

        virtual ~ComponentContainer() {}

        virtual const char* GetComponentContainerTypeName() const override
        {
            static const char* COMPONENT_TYPE_NAME{ typeid(T).name() };
            return COMPONENT_TYPE_NAME;
        }

        virtual void DestroyComponent(IComponent* object) override
        {
            // call d'tor
            object->~IComponent();
            this->DestroyObject(object);
        }

    }; // class ComponentContainer

public:
    template <class T>
    using TComponentIterator = typename ComponentContainer<T>::iterator;

    ComponentManager();
    ~ComponentManager();

private:
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(ComponentManager&) = delete;

public:
    /**
     * Adds a component of type T to entity described by entityId.
     * @tparam T - Generic type parameter.
     * @tparam ARGS - Type of the ...args.
     * @param entityId - Identifier for the entity.
     * @param args - Variable arguments providing [in,out] The arguments.
     * @return Null if it fails, else a pointer to a T.
     */
    template <class T, class... ARGS>
    T* AddComponent(const EntityId entityId, ARGS&&... args);

    /**
     * Removes the component of type T from an entity described by entityId.
     * @tparam T - Generic type parameter.
     * @param entityId - Identifier for the entity.
     */
    template <class T>
    void RemoveComponent(const EntityId entityId);

    void RemoveAllComponents(const EntityId entityId);

    /**
     * Get the component of type T of an entity. If component has no such
     * component nullptr is returned.
     * @tparam T - Generic type parameter.
     * @param entityId - Identifier for the entity.
     * @return Null if it fails, else the component.
     */
    template <class T>
    T* GetComponent(const EntityId entityId);

    /**
     * Returns an forward iterator object that points to the beginning of a
     * collection of all components of type T.
     * @tparam T - Generic type parameter.
     * @return A TComponentIterator&lt;T&gt;
     */
    template <class T>
    inline TComponentIterator<T> begin();

    /**
     * Returns an forward iterator object that points to the end of a collection
     * of all components of type T.
     * @tparam T - Generic type parameter.
     * @return A TComponentIterator&lt;T&gt;
     */
    template <class T>
    inline TComponentIterator<T> end();

private:
    template <class T>
    inline ComponentContainer<T>* GetComponentContainer();

    ComponentId AqcuireComponentId(IComponent* component);
    void        ReleaseComponentId(ComponentId id);

    void MapEntityComponent(EntityId        entityId,
                            ComponentId     componentId,
                            ComponentTypeId componentTypeId);
    void UnmapEntityComponent(EntityId        entityId,
                              ComponentId     componentId,
                              ComponentTypeId componentTypeId);

    using ComponentContainerRegistry =
        std::unordered_map<ComponentTypeId, IComponentContainer*>;
    ComponentContainerRegistry componentContainerRegistry;

    using ComponentLookupTable = std::vector<IComponent*>;
    ComponentLookupTable componentLookupTable;

    using EntityComponentMap = std::vector<std::vector<ComponentId>>;
    EntityComponentMap entityComponentMap;

}; // ComponentManager

} // namespace ecs

#endif // ECS_COMPONENTMANAGER_H
