#ifndef ECS_SYSTEMMANAGER_H
#define ECS_SYSTEMMANAGER_H

#include "api.h"
#include "engine.h"
#include "familytypeid.h"
#include "isystem.h"
#include "linearallocator.h"

namespace ecs
{
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
    // This class is not inteeded to be initialized
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(SystemManager&) = delete;

public:
    /**
     * Adds a new system.
     * @tparam T - Generic type parameter.
     * @tparam ARGS - Type of the arguments.
     * @param systemArgs - Variable arguments providing [in,out] The system
     * arguments.
     * @return Null if it fails, else a pointer to a T.
     */
    template <class T, class... ARGS>
    T* AddSystem(ARGS&&... systemArgs);

    /**
     * Adds a system dependency. This will result in an update of the dependency
     * matrix and re-creation of the systems working order. This method can be
     * quite costly so use it sparsely.
     * @tparam System_ - Type of the system.
     * @tparam Dependency_ - Type of the dependencies.
     * @param target - Target for the.
     * @param dependency - Variable arguments providing [in,out] The
     * dependencies.
     */
    template <class System_, class Dependency_>
    void AddSystemDependency(System_ target, Dependency_ dependency);

    template <class Target_, class Dependency_, class... Dependencies>
    void AddSystemDependency(Target_     target,
                             Dependency_ dependency,
                             Dependencies&&... dependencies);

    /**
     * Updates the work order of all systems. This should be called when changed
     * system priorities or adding new system dependencies.
     */
    void UpdateSystemWorkOrder();

    /**
     * Gets the system.
     * @tparam T - Generic type parameter.
     * @return Null if it fails, else the system.
     */
    template <class T>
    inline T* GetSystem() const;

    template <class T>
    void EnableSystem();

    template <class T>
    void DisableSystem();

    template <class T>
    void SetSystemUpdateInterval(f32 interval_ms);

    template <class T>
    void SetSystemPriority(SystemPriority newPriority);

    /**
     * Returns the current work state of all systems. The returned state mask
     * can be stored in local context and reset a later changed working state of
     * systems.
     * @return The system work state.
     */
    SystemWorkStateMask GetSystemWorkState() const;

    /**
     * Resets the current working state of systems by the provided state mask.
     * @param mask - The mask.
     */
    void SetSystemWorkState(SystemWorkStateMask mask);

    template <class... ActiveSystems>
    SystemWorkStateMask GenerateActiveSystemWorkState(
        ActiveSystems&&... activeSystems);
private:
    /**
     * Main update cycle.
     * @param dt_ms - The dt in milliseconds.
     */
    void Update(f32 dt_ms);

    SystemAllocator*       systemAllocator;
    SystemRegistry         systemRegistry;
    SystemDependencyMatrix systemDependencyMatrix;
    SystemWorkOrder        systemWorkOrder;
};

} // namespace ecs

#endif // ECS_SYSTEMMANAGER_H
