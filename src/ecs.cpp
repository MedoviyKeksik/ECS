#include "ecs.h"

namespace ecs
{
DEFINE_STATIC_LOGGER(IEntity, "Entity")

ComponentManager::ComponentManager()
{
    DEFINE_LOGGER("ComponentManager")
    LogInfo("Initialize ComponentManager!");

    const std::size_t numComponents{ util::internal::FamilyTypeID<IComponent>::Get() };

    this->entityComponentMap.resize(ENITY_LUT_GROW);
    for (auto i = 0; i < ENITY_LUT_GROW; ++i)
        this->entityComponentMap[i].resize(numComponents, INVALID_COMPONENT_ID);
}

ComponentManager::~ComponentManager()
{
    for (auto cc : this->componentContainerRegistry)
    {
        LogDebug("Releasing remaining entities of type '%s' ...", cc.second->GetComponentContainerTypeName());
        delete cc.second;
        cc.second = nullptr;
    }

    LogInfo("Release ComponentManager!");
}

void ComponentManager::RemoveAllComponents(const EntityId entityId)
{
    static const std::size_t numComponents = this->entityComponentMap[0].size();

    for (ComponentTypeId componentTypeId = 0; componentTypeId < numComponents; ++componentTypeId)
    {
        const ComponentId componentId = this->entityComponentMap[entityId.index][componentTypeId];
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

void ComponentManager::ReleaseComponentId(ComponentId id)
{
    assert((id != INVALID_COMPONENT_ID && id < this->componentLookupTable.size()) && "Invalid component id");
    this->componentLookupTable[id] = nullptr;
}

void ComponentManager::MapEntityComponent(EntityId entityId, ComponentId componentId, ComponentTypeId componentTypeId)
{
    static const std::size_t numComponents{ util::internal::FamilyTypeID<IComponent>::Get() };

    if ((this->entityComponentMap.size() - 1) < entityId.index)
    {
        std::size_t oldSize = this->entityComponentMap.size();

        // we scale this map size along the entity lookup table size
        std::size_t newSize = oldSize + ENITY_LUT_GROW;

        this->entityComponentMap.resize(newSize);

        for (auto i = oldSize; i < newSize; ++i)
            this->entityComponentMap[i].resize(numComponents, INVALID_COMPONENT_ID);
    }

    // create mapping
    this->entityComponentMap[entityId.index][componentTypeId] = componentId;
}

void ComponentManager::UnmapEntityComponent(EntityId entityId, ComponentId componentId, ComponentTypeId componentTypeId)
{
    assert(this->entityComponentMap[entityId.index][componentTypeId] == componentId &&
           "FATAL: Entity Component ID mapping corruption!");

    // free mapping
    this->entityComponentMap[entityId.index][componentTypeId] = INVALID_COMPONENT_ID;

    // free component id
    this->ReleaseComponentId(componentId);
}

IEntity::IEntity()
    : isActive(true)
    , entityId(INVALID_ENTITY_ID)
    , m_ComponentManagerInstance(new ComponentManager())
{
}

IEntity::IEntity(const EntityId& entityId, ComponentManager* componentManager)
    : isActive(true)
    , entityId(entityId)
    , m_ComponentManagerInstance(componentManager)
{
}

void IEntity::SetActive(bool active)
{
    if (this->isActive == active)
        return;
    if (active)
    {
        this->OnEnable();
    }
    else
    {
        this->OnDisable();
    }
    this->isActive = active;
}

EntityManager::EntityManager(ComponentManager* componentManagerInstance)
    : pendingDestroyedEntities(1024)
    , numPendingDestroyedEntities(0)
    , componentManager(componentManagerInstance){ DEFINE_LOGGER("EntityManager") LogInfo("InitializeEntityManager!") }

    EntityManager::~EntityManager()
{
    for (auto ec : this->entityRegistry)
    {
        LogDebug("Releasing remaining entities of type '%s' ...", ec.second->GetEntityContainerTypeName());
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
    for (std::size_t i = 0; i < this->numPendingDestroyedEntities; ++i)
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

void EntityManager::DestroyEntity(EntityId entityId)
{
    // maybe rudiment
    //    IEntity* entity = this->entityHandleTable[entityId];
    //    const EntityTypeId ETID = entity->GetStaticEntityTypeID();

    if (this->numPendingDestroyedEntities < this->pendingDestroyedEntities.size())
    {
        this->pendingDestroyedEntities[this->numPendingDestroyedEntities++] = entityId;
    }
    else
    {
        this->pendingDestroyedEntities.push_back(entityId);
        this->numPendingDestroyedEntities++;
    }
}

SystemManager::SystemManager()
{
    DEFINE_LOGGER("SystemManager")
    LogInfo("Initialize SystemManager!");

    // acquire global memory
    this->systemAllocator =
        new SystemAllocator(ECS_SYSTEM_MEMORY_BUFFER_SIZE, Allocate(ECS_SYSTEM_MEMORY_BUFFER_SIZE, "SystemManager"));
}

SystemManager::~SystemManager()
{
    for (SystemWorkOrder::reverse_iterator it = this->systemWorkOrder.rbegin(); it != this->systemWorkOrder.rend();
         ++it)
    {
        (*it)->~ISystem();
        *it = nullptr;
    }

    systemWorkOrder.clear();
    systemRegistry.clear();

    // free allocated global memory
    Free((void*)this->systemAllocator->GetMemoryAddress());
    delete this->systemAllocator;
    this->systemAllocator = nullptr;

    LogInfo("Release SystemManager!");
}

void SystemManager::Update(f32 dt_ms)
{
    for (ISystem* system : this->systemWorkOrder)
    {
        // increase interval since last update
        system->timeSinceLastUpdate += dt_ms;

        // check systems update state
        system->isNeedsUpdate =
            (system->updateInterval < 0.0f) ||
            ((system->updateInterval > 0.0f) && (system->timeSinceLastUpdate > system->updateInterval));

        if (system->isEnabled == true && system->isNeedsUpdate == true)
        {
            system->PreUpdate(dt_ms);
        }
    }

    for (ISystem* system : this->systemWorkOrder)
    {
        if (system->isEnabled == true && system->isNeedsUpdate == true)
        {
            system->Update(dt_ms);

            // reset interval
            system->timeSinceLastUpdate = 0.0f;
        }
    }

    for (ISystem* system : this->systemWorkOrder)
    {
        if (system->isEnabled == true && system->isNeedsUpdate == true)
        {
            system->PostUpdate(dt_ms);
        }
    }
}

void SystemManager::UpdateSystemWorkOrder()
{
    // depth-first-search function
    static const std::function<void(
        SystemTypeId, std::vector<int>&, const std::vector<std::vector<bool>>&, std::vector<SystemTypeId>&)>
        dfs = [&](SystemTypeId                          vertex,
                  std::vector<int>&                     vertexState,
                  const std::vector<std::vector<bool>>& edges,
                  std::vector<SystemTypeId>&            output)
    {
        vertexState[vertex] = 1; // visited

        for (int i = 0; i < vertexState.size(); ++i)
        {
            if (edges[i][vertex] == true && vertexState[i] == 0)
                dfs(i, vertexState, edges, output);
        }

        vertexState[vertex] = 2; // done
        output.push_back(vertex);
    };

    const std::size_t NUM_SYSTEMS = this->systemDependencyMatrix.size();

    // create index array
    std::vector<int> indices(NUM_SYSTEMS);
    for (int i = 0; i < NUM_SYSTEMS; ++i)
        indices[i] = i;

    // determine vertex-groups
    std::vector<std::vector<SystemTypeId>> vertexGroups;
    std::vector<SystemPriority>            groupPriority;

    while (indices.empty() == false)
    {
        SystemTypeId index = indices.back();
        indices.pop_back();

        if (index == -1)
            continue;

        std::vector<SystemTypeId> group;
        std::vector<SystemTypeId> member;

        SystemPriority currentGroupPriority = LOWEST_SYSTEM_PRIORITY;
        member.push_back(index);

        while (member.empty() == false)
        {
            index = member.back();
            member.pop_back();

            for (int i = 0; i < indices.size(); ++i)
            {
                if (indices[i] != -1 &&
                    (this->systemDependencyMatrix[i][index] == true || this->systemDependencyMatrix[index][i] == true))
                {
                    member.push_back(i);
                    indices[i] = -1;
                }
            }

            group.push_back(index);

            ISystem* sys = this->systemRegistry[index];
            currentGroupPriority =
                std::max((sys != nullptr ? sys->systemPriority : NORMAL_SYSTEM_PRIORITY), currentGroupPriority);
        }

        vertexGroups.push_back(group);
        groupPriority.push_back(currentGroupPriority);
    }

    const std::size_t numVertexGroups = vertexGroups.size();

    // do a topological sort on groups w.r.t. to groups priority!
    std::vector<int> vertex_states(NUM_SYSTEMS, 0);

    std::multimap<SystemPriority, std::vector<SystemTypeId>> vertexGroupsSorted;

    for (int i = 0; i < numVertexGroups; ++i)
    {
        auto g = vertexGroups[i];

        std::vector<SystemTypeId> order;

        for (int j = 0; j < g.size(); ++j)
        {
            if (vertex_states[g[j]] == 0)
                dfs(g[j], vertex_states, this->systemDependencyMatrix, order);
        }

        std::reverse(order.begin(), order.end());

        // note: MAX - PRIORITY will frce the correct sorting behaviour in
        // multimap (by default a multimap sorts int values from low to high)
        vertexGroupsSorted.insert(std::pair<SystemPriority, std::vector<SystemTypeId>>(
            std::numeric_limits<SystemPriority>::max() - groupPriority[i], order));
    }

    LogInfo("Update system work order:")

        // re-build system work order
        this->systemWorkOrder.clear();
    for (const auto& group : vertexGroupsSorted)
    {
        for (const auto& m : group.second)
        {
            ISystem* sys = this->systemRegistry[m];
            if (sys != nullptr)
            {
                this->systemWorkOrder.push_back(sys);
                LogInfo("\t%s", sys->GetSystemTypeName())
            }
        }
    }
}

SystemWorkStateMask SystemManager::GetSystemWorkState() const
{
    SystemWorkStateMask mask(this->systemWorkOrder.size());

    for (int i = 0; i < this->systemWorkOrder.size(); ++i)
    {
        mask[i] = this->systemWorkOrder[i]->isEnabled;
    }

    return mask;
}

void SystemManager::SetSystemWorkState(SystemWorkStateMask mask)
{
    assert(mask.size() == this->systemWorkOrder.size() && "Provided mask does not match size of current system array.");

    for (int i = 0; i < this->systemWorkOrder.size(); ++i)
    {
        this->systemWorkOrder[i]->isEnabled = mask[i];
    }
}

} // namespace ecs
