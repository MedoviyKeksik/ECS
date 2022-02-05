#pragma once

//#include "api.h"
#include "util/family_type_id.h"
#include "util/handle.h"

#include "memory/allocators/linear_allocator.h"
#include "memory/memory_chunk_allocator.h"

namespace ecs
{
using EntityTypeId = TypeID;
using EntityId     = util::Handle64;

static const EntityId INVALID_ENTITY_ID = util::Handle64::INVALID_HANDLE;

using ComponentId     = ObjectID;
using ComponentTypeId = TypeID;

static const ComponentId INVALID_COMPONENT_ID = INVALID_OBJECT_ID;

class ECS_API IComponent
{
    friend class ComponentManager;

public:
    IComponent()
        : owner(INVALID_ENTITY_ID)
        , enabled(true)
    {
    }
    virtual ~IComponent() = default;

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

    inline void SetIsActive(bool state) { this->enabled = state; }

    inline bool GetIsActive() const { return this->enabled; }

protected:
    ComponentId hashValue;
    ComponentId componentId;
    EntityId    owner;
    bool        enabled;
};

template <typename T>
class ECS_API Component : public IComponent
{
public:
    static const ComponentTypeId STATIC_COMPONENT_TYPE_ID;

    Component() = default;

    virtual ~Component() = default;

    inline ComponentTypeId GetStaticComponentTypeID() const
    {
        return STATIC_COMPONENT_TYPE_ID;
    }
};

// This private member only exists to force the compiler to create an instance
// of Component T, which will set its unique identifier.
template <typename T>
const ComponentTypeId Component<T>::STATIC_COMPONENT_TYPE_ID =
    util::internal::FamilyTypeID<IComponent>::Get<T>();

class ECS_API ComponentManager : memory::GlobalMemoryUser
{
    friend class IComponent;
    DECLARE_LOGGER

    class IComponentContainer
    {
    public:
        virtual ~IComponentContainer() = default;

        virtual const char* GetComponentContainerTypeName() const = 0;

        virtual void DestroyComponent(IComponent* object) = 0;
    };

    template <typename T>
    class ComponentContainer
        : public memory::MemoryChunkAllocator<T, COMPONENT_T_CHUNK_SIZE>,
          public IComponentContainer
    {
        ComponentContainer(const ComponentContainer&) = delete;
        ComponentContainer& operator=(ComponentContainer&) = delete;

    public:
        ComponentContainer()
            : memory::MemoryChunkAllocator<T, COMPONENT_T_CHUNK_SIZE>(
                  "ComponentManager")
        {
        }

        virtual ~ComponentContainer() = default;

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
    template <typename T>
    using TComponentIterator = typename ComponentContainer<T>::iterator;

    ComponentManager();
    ~ComponentManager();

private:
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(ComponentManager&) = delete;

public:
    template <typename T, class... ARGS>
    T* AddComponent(const EntityId entityId, ARGS&&... args)
    {
        // hash operator for hashing entity and component ids
        static constexpr std::hash<ComponentId> entityComponentIdHasher{
            std::hash<ComponentId>()
        };

        const ComponentTypeId CTID = T::STATIC_COMPONENT_TYPE_ID;

        // aqcuire memory for new component object of type T
        void* pObjectMemory = GetComponentContainer<T>()->CreateObject();

        ComponentId componentId = this->AqcuireComponentId((T*)pObjectMemory);
        ((T*)pObjectMemory)->componentId = componentId;

        // create component inplace
        IComponent* component =
            new (pObjectMemory) T(std::forward<ARGS>(args)...);

        component->owner     = entityId;
        component->hashValue = entityComponentIdHasher(entityId) ^
                               (entityComponentIdHasher(componentId) << 1);

        // create mapping from entity id its component id
        MapEntityComponent(entityId, componentId, CTID);

        return static_cast<T*>(component);
    }

    template <typename T>
    void RemoveComponent(const EntityId entityId)
    {
        const ComponentTypeId componentTypeId = T::STATIC_COMPONENT_TYPE_ID;

        const ComponentId componentId =
            this->entityComponentMap[entityId.index][componentTypeId];

        IComponent* component = this->componentLookupTable[componentId];

        assert(component != nullptr && "FATAL: Trying to remove a component "
                                       "which is not used by this entity!");

        // release object memory
        GetComponentContainer<T>()->DestroyObject(component);

        // unmap entity id to component id
        UnmapEntityComponent(entityId, componentId, componentTypeId);
    }

    void RemoveAllComponents(const EntityId entityId);

    template <typename T>
    T* GetComponent(const EntityId entityId)
    {
        const ComponentTypeId componentTypeId = T::STATIC_COMPONENT_TYPE_ID;

        const ComponentId componentId =
            this->entityComponentMap[entityId.index][componentTypeId];

        // entity has no component of type T
        if (componentId == INVALID_COMPONENT_ID)
            return nullptr;

        return static_cast<T*>(this->componentLookupTable[componentId]);
    }

    template <typename T>
    inline TComponentIterator<T> begin()
    {
        return GetComponentContainer<T>()->begin();
    }

    template <typename T>
    inline TComponentIterator<T> end()
    {
        return GetComponentContainer<T>()->end();
    }

private:
    template <typename T>
    inline ComponentContainer<T>* GetComponentContainer()
    {
        ComponentTypeId componentTypeId = T::STATIC_COMPONENT_TYPE_ID;

        auto it = this->componentContainerRegistry.find(componentTypeId);
        ComponentContainer<T>* cc = nullptr;

        if (it == this->componentContainerRegistry.end())
        {
            cc = new ComponentContainer<T>();
            this->componentContainerRegistry[componentTypeId] = cc;
        }
        else
            cc = static_cast<ComponentContainer<T>*>(it->second);

        assert(cc != nullptr && "Failed to create ComponentContainer<T>!");
        return cc;
    }

    ComponentId AqcuireComponentId(IComponent* component)
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

    void ReleaseComponentId(ComponentId id);

    void MapEntityComponent(EntityId        entityId,
                            ComponentId     componentId,
                            ComponentTypeId componentTypeId);

    void UnmapEntityComponent(EntityId        entityId,
                              ComponentId     componentId,
                              ComponentTypeId componentTypeId);

private:
    using ComponentContainerRegistry =
        std::unordered_map<ComponentTypeId, IComponentContainer*>;
    ComponentContainerRegistry componentContainerRegistry;

    using ComponentLookupTable = std::vector<IComponent*>;
    ComponentLookupTable componentLookupTable;

    using EntityComponentMap = std::vector<std::vector<ComponentId>>;
    EntityComponentMap entityComponentMap;

}; // ComponentManager

class ECS_API IEntity
{
    friend class EntityManager;

public:
    IEntity();
    virtual ~IEntity() = default;

    template <typename T>
    T* GetComponent() const
    {
        return this->m_ComponentManagerInstance->GetComponent<T>(
            this->entityId);
    }

    template <typename T, class... P>
    T* AddComponent(P&&... param)
    {
        return this->m_ComponentManagerInstance->AddComponent<T>(
            this->entityId, std::forward<P>(param)...);
    }

    template <typename T>
    void RemoveComponent()
    {
        this->m_ComponentManagerInstance->RemoveComponent<T>(this->entityId);
    }

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
    bool     isActive; // if false, entity won't be updated
};

template <typename E>
class ECS_API Entity : public IEntity
{
public:
    static const EntityTypeId STATIC_ENTITY_TYPE_ID;

    Entity() = default;

    virtual ~Entity() = default;

private:
    // Entity destruction always happens through EntityManager !!!
    void operator delete(void*)   = delete;
    void operator delete[](void*) = delete;

public:
    virtual const EntityTypeId GetStaticEntityTypeID() const override
    {
        return STATIC_ENTITY_TYPE_ID;
    }
};

// set unique type id for this Entity<T>
template <typename E>
const EntityTypeId Entity<E>::STATIC_ENTITY_TYPE_ID =
    util::internal::FamilyTypeID<IEntity>::Get<E>();

using EntityHandleTable = util::HandleTable<IEntity, EntityId>;

class ECS_API EntityManager
{
    DECLARE_LOGGER

    class IEntityContainer
    {
    public:
        virtual ~IEntityContainer() = default;

        virtual const char* GetEntityContainerTypeName() const = 0;

        virtual void DestroyEntity(IEntity* object) = 0;
    }; // class IEntityContainer

    template <typename T>
    class EntityContainer
        : public memory::MemoryChunkAllocator<T, ENITY_T_CHUNK_SIZE>,
          public IEntityContainer
    {
    public:
        EntityContainer()
            : memory::MemoryChunkAllocator<T, ENITY_T_CHUNK_SIZE>(
                  "EntityManager")
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
    template <typename T, class... ARGS>
    EntityId CreateEntity(ARGS&&... args)
    {
        // aqcuire memory for new entity object of type T
        void* pObjectMemory = GetEntityContainer<T>()->CreateObject();

        ecs::EntityId entityId = this->AqcuireEntityId((T*)pObjectMemory);

        ((T*)pObjectMemory)->entityId = entityId;
        ((T*)pObjectMemory)->m_ComponentManagerInstance =
            this->componentManager;

        // create entity inplace
        IEntity* entity = new (pObjectMemory) T(std::forward<ARGS>(args)...);

        return entityId;
    }

    void DestroyEntity(EntityId entityId);

    inline IEntity* GetEntity(EntityId entityId);

    inline EntityId GetEntityId(EntityId::value_type index) const;

    void RemoveDestroyedEntities();

private:
    template <typename T>
    inline EntityContainer<T>* GetEntityContainer()
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

    EntityId AqcuireEntityId(IEntity* entity);

    void ReleaseEntityId(EntityId id);

private:
    using EntityRegistry = std::unordered_map<EntityTypeId, IEntityContainer*>;
    using PendingDestroyedEntities = std::vector<EntityId>;
    EntityRegistry           entityRegistry;
    PendingDestroyedEntities pendingDestroyedEntities;
    std::size_t              numPendingDestroyedEntities;
    ComponentManager*        componentManager;
    EntityHandleTable        entityHandleTable;
};

template <typename T>
class System;

using SystemTypeId   = TypeID;
using SystemPriority = u16;

static const SystemTypeId   INVALID_SYSTEMID = INVALID_TYPE_ID;
static const SystemPriority LOWEST_SYSTEM_PRIORITY =
    std::numeric_limits<SystemPriority>::min();
static const SystemPriority VERY_LOW_SYSTEM_PRIORITY  = 99;
static const SystemPriority LOW_SYSTEM_PRIORITY       = 100;
static const SystemPriority NORMAL_SYSTEM_PRIORITY    = 200;
static const SystemPriority MEDIUM_SYSTEM_PRIORITY    = 300;
static const SystemPriority HIGH_SYSTEM_PRIORITY      = 400;
static const SystemPriority VERY_HIGH_SYSTEM_PRIORITY = 401;
static const SystemPriority HIGHEST_SYSTEM_PRIORITY =
    std::numeric_limits<SystemPriority>::max();

class ECS_API ISystem
{
    friend class SystemManager;

protected:
    ISystem(SystemPriority priority          = NORMAL_SYSTEM_PRIORITY,
            f32            updateInterval_ms = -1.0f)
        : systemPriority(priority)
        , updateInterval(updateInterval_ms)
        , timeSinceLastUpdate()
        , isNeedsUpdate()
        , reserved()
        , isEnabled(true)
    {
    }

public:
    virtual ~ISystem() = default;

    virtual inline const SystemTypeId GetStaticSystemTypeID() const = 0;
    virtual inline const char*        GetSystemTypeName() const     = 0;

    virtual void PreUpdate(f32 dt)  = 0;
    virtual void Update(f32 dt)     = 0;
    virtual void PostUpdate(f32 dt) = 0;

private:
    f32            timeSinceLastUpdate;
    SystemPriority systemPriority;
    f32            updateInterval;
    u8             isEnabled : 1;
    u8             isNeedsUpdate : 1;
    u8             reserved : 6;
};

using SystemWorkStateMask = std::vector<bool>;

class ECS_API SystemManager : memory::GlobalMemoryUser
{
    friend EcsEngine;
    DECLARE_LOGGER

    using SystemDependencyMatrix = std::vector<std::vector<bool>>;
    using SystemRegistry         = std::unordered_map<u64, ISystem*>;
    using SystemAllocator        = memory::allocator::LinearAllocator;
    using SystemWorkOrder        = std::vector<ISystem*>;

public:
    SystemManager();
    ~SystemManager();

private:
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(SystemManager&) = delete;

public:
    template <typename T, class... ARGS>
    T* AddSystem(ARGS&&... systemArgs)
    {
        const u64 staticSystemTypeId = T::STATIC_SYSTEM_TYPE_ID;

        // avoid multiple registrations of the same system
        auto it = this->systemRegistry.find(staticSystemTypeId);
        if ((this->systemRegistry.find(staticSystemTypeId) !=
             this->systemRegistry.end()) &&
            (it->second != nullptr))
            return (T*)it->second;

        T*    system = nullptr;
        void* pSystemMem =
            this->systemAllocator->Allocate(sizeof(T), alignof(T));
        if (pSystemMem != nullptr)
        {

            ((T*)pSystemMem)->systemManager = this;

            // create new system
            system = new (pSystemMem) T(std::forward<ARGS>(systemArgs)...);
            this->systemRegistry[staticSystemTypeId] = system;

            LogInfo("System \'%s\' (%d bytes) created.",
                    typeid(T).name(),
                    sizeof(T));
        }
        else
        {
            LogError("Unable to create system \'%s\' (%d bytes).",
                     typeid(T).name(),
                     sizeof(T));
            assert(true);
        }

        // resize dependency matrix
        if (staticSystemTypeId + 1 > this->systemDependencyMatrix.size())
        {
            this->systemDependencyMatrix.resize(staticSystemTypeId + 1);
            for (int i = 0; i < this->systemDependencyMatrix.size(); ++i)
                this->systemDependencyMatrix[i].resize(staticSystemTypeId + 1);
        }

        // add to work list
        this->systemWorkOrder.push_back(system);

        return system;
    }

    template <typename System_, class Dependency_>
    void AddSystemDependency(System_ target, Dependency_ dependency)
    {
        const u64 TARGET_ID = target->GetStaticSystemTypeID();
        const u64 DEPEND_ID = dependency->GetStaticSystemTypeID();

        if (this->systemDependencyMatrix[TARGET_ID][DEPEND_ID] != true)
        {
            this->systemDependencyMatrix[TARGET_ID][DEPEND_ID] = true;
            LogInfo("added '%s' as dependency to '%s'",
                    dependency->GetSystemTypeName(),
                    target->GetSystemTypeName())
        }

        // this->UpdateSystemWorkOrder();
    }

    template <typename Target_, class Dependency_, class... Dependencies>
    void AddSystemDependency(Target_     target,
                             Dependency_ dependency,
                             Dependencies&&... dependencies)
    {
        const u64 TARGET_ID = target->GetStaticSystemTypeID();
        const u64 DEPEND_ID = dependency->GetStaticSystemTypeID();

        if (this->systemDependencyMatrix[TARGET_ID][DEPEND_ID] != true)
        {
            this->systemDependencyMatrix[TARGET_ID][DEPEND_ID] = true;
            LogInfo("added '%s' as dependency to '%s'",
                    dependency->GetSystemTypeName(),
                    target->GetSystemTypeName())
        }

        this->AddSystemDependency(target,
                                  std::forward<Dependencies>(dependencies)...);
    }

    void UpdateSystemWorkOrder();

    template <typename T>
    inline T* GetSystem() const
    {
        auto it = this->systemRegistry.find(T::STATIC_SYSTEM_TYPE_ID);
        return it != this->systemRegistry.end() ? (T*)it->second : nullptr;
    }

    template <typename T>
    void EnableSystem()
    {
        const SystemTypeId STID = T::STATIC_SYSTEM_TYPE_ID;
        // get system
        auto it = this->systemRegistry.find(STID);
        if (it != this->systemRegistry.end())
        {
            if (it->second->isEnabled == true)
                return;
            // enable system
            it->second->isEnabled = true;
        }
        else
        {
            LogWarning("Trying to enable system [%d], but system is not "
                       "registered yet.",
                       STID);
        }
    }

    template <typename T>
    void DisableSystem()
    {
        const SystemTypeId STID = T::STATIC_SYSTEM_TYPE_ID;
        // get system
        auto it = this->systemRegistry.find(STID);
        if (it != this->systemRegistry.end())
        {
            if (it->second->isEnabled == false)
                return;

            // enable system
            it->second->isEnabled = false;
        }
        else
        {
            LogWarning("Trying to disable system [%d], but system is not "
                       "registered yet.",
                       STID);
        }
    }

    template <typename T>
    void SetSystemUpdateInterval(f32 interval_ms)
    {
        const SystemTypeId STID = T::STATIC_SYSTEM_TYPE_ID;
        // get system
        auto it = this->systemRegistry.find(STID);
        if (it != this->systemRegistry.end())
        {
            it->second->updateInterval = interval_ms;
        }
        else
        {
            LogWarning("Trying to change system's [%d] update interval, but "
                       "system is not registered yet.",
                       STID);
        }
    }

    template <typename T>
    void SetSystemPriority(SystemPriority newPriority)
    {
        const SystemTypeId STID = T::STATIC_SYSTEM_TYPE_ID;
        // get system
        auto it = this->systemRegistry.find(STID);
        if (it != this->systemRegistry.end())
        {
            SystemPriority oldPriority = it->second->systemPriority;

            if (oldPriority == newPriority)
                return;

            it->second->systemPriority = newPriority;

            // re-build system work order
            // this->UpdateSystemWorkOrder();
        }
        else
        {
            LogWarning("Trying to change system's [%d] priority, but system is "
                       "not registered yet.",
                       STID);
        }
    }

    SystemWorkStateMask GetSystemWorkState() const;

    void SetSystemWorkState(SystemWorkStateMask mask);

    template <typename... ActiveSystems>
    SystemWorkStateMask GenerateActiveSystemWorkState(
        ActiveSystems&&... activeSystems)
    {
        SystemWorkStateMask mask(this->systemWorkOrder.size(), false);
        std::list<ISystem*> AS = { activeSystems... };
        for (auto s : AS)
        {
            for (int i = 0; i < this->systemWorkOrder.size(); ++i)
            {
                if (this->systemWorkOrder[i]->GetStaticSystemTypeID() ==
                    s->GetStaticSystemTypeID())
                {
                    mask[i] = true;
                    break;
                }
            }
        }
        return mask;
    }

private:
    void Update(f32 dt_ms);

    SystemAllocator*       systemAllocator;
    SystemRegistry         systemRegistry;
    SystemDependencyMatrix systemDependencyMatrix;
    SystemWorkOrder        systemWorkOrder;
};

template <typename T>
class ECS_API System : public ISystem
{
    friend class SystemManager;

protected:
    DECLARE_LOGGER
    System()
    {
        DEFINE_LOGGER(typeid(T).name())
        LogInfo("System %s created.", typeid(T).name());
    }

public:
    virtual ~System() { LogInfo("System %s released.", typeid(T).name()); }

    static const SystemTypeId STATIC_SYSTEM_TYPE_ID;

    virtual inline const SystemTypeId GetStaticSystemTypeID() const
    {
        return STATIC_SYSTEM_TYPE_ID;
    }

    virtual inline const char* GetSystemTypeName() const override
    {
        static const char* SYSTEM_TYPE_NAME{ typeid(T).name() };
        return SYSTEM_TYPE_NAME;
    }

    template <typename... Dependencies>
    void AddDependencies(Dependencies&&... dependencies)
    {
        this->systemManager->AddSystemDependency(
            this, std::forward<Dependencies>(dependencies)...);
    }

    virtual void PreUpdate(f32 dt) override {}

    virtual void Update(f32 dt) override {}

    virtual void PostUpdate(f32 dt) override {}

private:
    SystemManager* systemManager;

}; // class System<T>

template <typename T>
const SystemTypeId System<T>::STATIC_SYSTEM_TYPE_ID =
    util::internal::FamilyTypeID<ISystem>::Get<T>();

} // namespace ecs
