#include "componentmanager.h"

namespace ecs
{

ComponentManager::ComponentManager()
{
    DEFINE_LOGGER("ComponentManager")
    LogInfo("Initialize ComponentManager!");

    const size_t NUM_COMPONENTS{
        util::internal::FamilyTypeID<IComponent>::Get()
    };

    this->entityComponentMap.resize(ENITY_LUT_GROW);
    for (auto i = 0; i < ENITY_LUT_GROW; ++i)
        this->entityComponentMap[i].resize(NUM_COMPONENTS,
                                             INVALID_COMPONENT_ID);
}

ComponentManager::~ComponentManager()
{
    for (auto cc : this->componentContainerRegistry)
    {
        LogDebug("Releasing remaining entities of type '%s' ...",
                 cc.second->GetComponentContainerTypeName());
        delete cc.second;
        cc.second = nullptr;
    }

    LogInfo("Release ComponentManager!");
}

ComponentId ComponentManager::AqcuireComponentId(IComponent* component)
{
    int i = 0;
    for (; i < this->componentLookupTable.size(); ++i)
    {
        if (this->componentLookupTable[i] == nullptr)
        {
            this->componentLookupTable[i] = component;
            return i;
        }
    }

    // increase component LUT size
    this->componentLookupTable.resize(
        this->componentLookupTable.size() + COMPONENT_LUT_GROW, nullptr);

    this->componentLookupTable[i] = component;
    return i;
}

void ComponentManager::ReleaseComponentId(ComponentId id)
{
    assert((id != INVALID_COMPONENT_ID && id < this->componentLookupTable.size()) &&
           "Invalid component id");
    this->componentLookupTable[id] = nullptr;
}

void ComponentManager::MapEntityComponent(EntityId        entityId,
                                          ComponentId     componentId,
                                          ComponentTypeId componentTypeId)
{
    static const size_t NUM_COMPONENTS{
        util::Internal::FamilyTypeID<IComponent>::Get()
    };

    if ((this->entityComponentMap.size() - 1) < entityId.index)
    {
        size_t oldSize = this->entityComponentMap.size();

        // we scale this map size along the entity lookup table size
        size_t newSize = oldSize + ENITY_LUT_GROW;

        this->entityComponentMap.resize(newSize);

        for (auto i = oldSize; i < newSize; ++i)
            this->entityComponentMap[i].resize(NUM_COMPONENTS,
                                                 INVALID_COMPONENT_ID);
    }

    // create mapping
    this->entityComponentMap[entityId.index][componentTypeId] = componentId;
}

void ComponentManager::UnmapEntityComponent(EntityId        entityId,
                                            ComponentId     componentId,
                                            ComponentTypeId componentTypeId)
{
    assert(this->entityComponentMap[entityId.index][componentTypeId] ==
               componentId &&
           "FATAL: Entity Component ID mapping corruption!");

    // free mapping
    this->entityComponentMap[entityId.index][componentTypeId] =
        INVALID_COMPONENT_ID;

    // free component id
    this->ReleaseComponentId(componentId);
}

template <class T, class... ARGS>
T* ComponentManager::AddComponent(const EntityId entityId, ARGS&&... args)
{
    // hash operator for hashing entity and component ids
    static constexpr std::hash<ComponentId> ENTITY_COMPONENT_ID_HASHER{
        std::hash<ComponentId>()
    };

    const ComponentTypeId CTID = T::STATIC_COMPONENT_TYPE_ID;

    // aqcuire memory for new component object of type T
    void* pObjectMemory = GetComponentContainer<T>()->CreateObject();

    ComponentId componentId = this->AqcuireComponentId((T*)pObjectMemory);
    ((T*)pObjectMemory)->componentId = componentId;

    // create component inplace
    IComponent* component = new (pObjectMemory) T(std::forward<ARGS>(args)...);

    component->owner       = entityId;
    component->hashValue   = ENTITY_COMPONENT_ID_HASHER(entityId) ^
                             (ENTITY_COMPONENT_ID_HASHER(componentId) << 1);

    // create mapping from entity id its component id
    MapEntityComponent(entityId, componentId, CTID);

    return static_cast<T*>(component);
}

template <class T>
void ComponentManager::RemoveComponent(const EntityId entityId)
{
    const ComponentTypeId CTID = T::STATIC_COMPONENT_TYPE_ID;

    const ComponentId componentId =
        this->entityComponentMap[entityId.index][CTID];

    IComponent* component = this->componentLookupTable[componentId];

    assert(component != nullptr && "FATAL: Trying to remove a component "
                                   "which is not used by this entity!");

    // release object memory
    GetComponentContainer<T>()->DestroyObject(component);

    // unmap entity id to component id
    UnmapEntityComponent(entityId, componentId, CTID);
}

void ComponentManager::RemoveAllComponents(const EntityId entityId)
{
    static const size_t NUM_COMPONENTS = this->entityComponentMap[0].size();

    for (ComponentTypeId componentTypeId = 0; componentTypeId < NUM_COMPONENTS;
         ++componentTypeId)
    {
        const ComponentId componentId =
            this->entityComponentMap[entityId.index][componentTypeId];
        if (componentId == INVALID_COMPONENT_ID)
            continue;

        IComponent* component = this->componentLookupTable[componentId];
        if (component != nullptr)
        {
            // get appropriate component container
            auto it = this->componentContainerRegistry.find(componentTypeId);
            if (it != this->componentContainerRegistry.end())
                it->second->DestroyComponent(component);
            else
                assert(false && "Trying to release a component that wasn't "
                                "created by ComponentManager!");

            // unmap entity id to component id
            UnmapEntityComponent(entityId, componentId, componentTypeId);
        }
    }
}

template <class T>
T* ComponentManager::GetComponent(const EntityId entityId)
{
    const ComponentTypeId CTID = T::STATIC_COMPONENT_TYPE_ID;

    const ComponentId componentId =
        this->entityComponentMap[entityId.index][CTID];

    // entity has no component of type T
    if (componentId == INVALID_COMPONENT_ID)
        return nullptr;

    return static_cast<T*>(this->componentLookupTable[componentId]);
}

template <class T>
ComponentManager::TComponentIterator<T> ComponentManager::begin()
{
    return GetComponentContainer<T>()->begin();
}

template <class T>
ComponentManager::TComponentIterator<T> ComponentManager::end()
{
    return GetComponentContainer<T>()->end();
}

template <class T>
ComponentManager::ComponentContainer<T>*
ComponentManager::GetComponentContainer()
{

    ComponentTypeId CID = T::STATIC_COMPONENT_TYPE_ID;

    auto                   it = this->componentContainerRegistry.find(CID);
    ComponentContainer<T>* cc = nullptr;

    if (it == this->componentContainerRegistry.end())
    {
        cc                                    = new ComponentContainer<T>();
        this->componentContainerRegistry[CID] = cc;
    }
    else
        cc = static_cast<ComponentContainer<T>*>(it->second);

    assert(cc != nullptr && "Failed to create ComponentContainer<T>!");
    return cc;
}

} // namespace ecs