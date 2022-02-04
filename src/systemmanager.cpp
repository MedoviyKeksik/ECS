#include "systemmanager.h"
#include "isystem.h"

namespace ecs
{
SystemManager::SystemManager()
{
    DEFINE_LOGGER("SystemManager")
    LogInfo("Initialize SystemManager!");

    // acquire global memory
    this->systemAllocator = new SystemAllocator(
        ECS_SYSTEM_MEMORY_BUFFER_SIZE,
        Allocate(ECS_SYSTEM_MEMORY_BUFFER_SIZE, "SystemManager"));
}

SystemManager::~SystemManager()
{
    for (SystemWorkOrder::reverse_iterator it = this->systemWorkOrder.rbegin();
         it != this->systemWorkOrder.rend();
         ++it)
    {
        (*it)->~ISystem();
        *it = nullptr;
    }

    systemWorkOrder.clear();
    systemRegistry.clear();

    // free allocated global memory
    Free((void*)this->systemAllocator->GetMemoryAddress0());
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
            ((system->updateInterval > 0.0f) &&
             (system->timeSinceLastUpdate > system->updateInterval));

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
    static const std::function<void(SystemTypeId,
                                    std::vector<int>&,
                                    const std::vector<std::vector<bool>>&,
                                    std::vector<SystemTypeId>&)>
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

    const size_t NUM_SYSTEMS = this->systemDependencyMatrix.size();

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
                    (this->systemDependencyMatrix[i][index] == true ||
                     this->systemDependencyMatrix[index][i] == true))
                {
                    member.push_back(i);
                    indices[i] = -1;
                }
            }

            group.push_back(index);

            ISystem* sys         = this->systemRegistry[index];
            currentGroupPriority = std::max(
                (sys != nullptr ? sys->systemPriority : NORMAL_SYSTEM_PRIORITY),
                currentGroupPriority);
        }

        vertexGroups.push_back(group);
        groupPriority.push_back(currentGroupPriority);
    }

    const size_t numVertexGroups = vertexGroups.size();

    // do a topological sort on groups w.r.t. to groups priority!
    std::vector<int> vertex_states(NUM_SYSTEMS, 0);

    std::multimap<SystemPriority, std::vector<SystemTypeId>> vertexGroupsSorted;

    for (int i = 0; i < numVertexGroups; ++i)
    {
        auto g = vertexGroups[i];

        std::vector<SystemTypeId> order;

        for (int i = 0; i < g.size(); ++i)
        {
            if (vertex_states[g[i]] == 0)
                dfs(g[i], vertex_states, this->systemDependencyMatrix, order);
        }

        std::reverse(order.begin(), order.end());

        // note: MAX - PRIORITY will frce the correct sorting behaviour in
        // multimap (by default a multimap sorts int values from low to high)
        vertexGroupsSorted.insert(
            std::pair<SystemPriority, std::vector<SystemTypeId>>(
                std::numeric_limits<SystemPriority>::max() - groupPriority[i],
                order));
    }

    LogInfo("Update system work order:")

        // re-build system work order
        this->systemWorkOrder.clear();
    for (auto group : vertexGroupsSorted)
    {
        for (auto m : group.second)
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
    assert(mask.size() == this->systemWorkOrder.size() &&
           "Provided mask does not match size of current system array.");

    for (int i = 0; i < this->systemWorkOrder.size(); ++i)
    {
        this->systemWorkOrder[i]->isEnabled = mask[i];
    }
}

template <class T, class... ARGS>
T* SystemManager::AddSystem(ARGS&&... systemArgs)
{
    const u64 staticSystemTypeId = T::STATIC_SYSTEM_TYPE_ID;

    // avoid multiple registrations of the same system
    auto it = this->systemRegistry.find(staticSystemTypeId);
    if ((this->systemRegistry.find(staticSystemTypeId) != this->systemRegistry.end()) &&
        (it->second != nullptr))
        return (T*)it->second;

    T*    system     = nullptr;
    void* pSystemMem = this->systemAllocator->allocate(sizeof(T), alignof(T));
    if (pSystemMem != nullptr)
    {

        ((T*)pSystemMem)->systemManager = this;

        // create new system
        system = new (pSystemMem) T(std::forward<ARGS>(systemArgs)...);
        this->systemRegistry[staticSystemTypeId] = system;

        LogInfo(
            "System \'%s\' (%d bytes) created.", typeid(T).name(), sizeof(T));
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
template <class System_, class Dependency_>
void SystemManager::AddSystemDependency(System_ target, Dependency_ dependency)
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
template <class Target_, class Dependency_, class... Dependencies>
void SystemManager::AddSystemDependency(Target_     target,
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
template <class T>
void SystemManager::EnableSystem()
{
    const SystemTypeId STID = T::STATIC_SYSTEM_TYPE_ID;
    // get system
    auto it = this->systemRegistry.find(STID);
    if (it != this->systemRegistry.end())
    {
        if (it->second->enabled == true)
            return;
        // enable system
        it->second->enabled = true;
    }
    else
    {
        LogWarning("Trying to enable system [%d], but system is not "
                   "registered yet.",
                   STID);
    }
}
template <class T>
void SystemManager::DisableSystem()
{
    const SystemTypeId STID = T::STATIC_SYSTEM_TYPE_ID;
    // get system
    auto it = this->systemRegistry.find(STID);
    if (it != this->systemRegistry.end())
    {
        if (it->second->enabled == false)
            return;

        // enable system
        it->second->enabled = false;
    }
    else
    {
        LogWarning("Trying to disable system [%d], but system is not "
                   "registered yet.",
                   STID);
    }
}
template <class T>
void SystemManager::SetSystemUpdateInterval(f32 interval_ms)
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
template <class T>
void SystemManager::SetSystemPriority(SystemPriority newPriority)
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
template <class T>
T* SystemManager::GetSystem() const
{
    auto it = this->systemRegistry.find(T::STATIC_SYSTEM_TYPE_ID);
    return it != this->systemRegistry.end() ? (T*)it->second : nullptr;
}
template <class... ActiveSystems>
SystemWorkStateMask SystemManager::GenerateActiveSystemWorkState(
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

} // namespace ecs
