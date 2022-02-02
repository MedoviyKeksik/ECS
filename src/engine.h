#pragma once

#include "api.h"
#include "eventdelegate.h"
#include "eventhandler.h"

namespace ecs
{
namespace util
{
class Timer;
}
namespace event
{
class IEvent;
class IEventListener;
class EventHadler;
} // namespace event

class EntityManager;
class SystemManager;
class ComponentManager;

class ECS_API EcsEngine
{
    friend class IEntity;
    friend class IComponent;
    friend class ISystem;

    friend class event::IEvent;
    friend class event::IEventListener;

    friend class EntityManager;

private:
    util::Timer*         ECS_EngineTime;
    EntityManager*       ECS_EntityManager;
    ComponentManager*    ECS_ComponentManager;
    SystemManager*       ECS_SystemManager;
    event::EventHandler* ECS_EventHandler;

    EcsEngine(const EcsEngine&) = delete;
    EcsEngine& operator=(EcsEngine&) = delete;

    // Add event callback
    template <class E>
    inline void SubscribeEvent(
        event::internal::IEventDelegate* const eventDelegate)
    {
        ECS_EventHandler->AddEventCallback<E>(eventDelegate);
    }

    // Remove event callback
    inline void UnsubscribeEvent(
        event::internal::IEventDelegate* eventDelegate);

public:
    EcsEngine();
    ~EcsEngine();

    inline EntityManager*    GetEntityManager() { return ECS_EntityManager; }
    inline ComponentManager* GetComponentManager()
    {
        return ECS_ComponentManager;
    }
    inline SystemManager* GetSystemManager() { return ECS_SystemManager; }

    /**
     * Broadcasts an event.
     * @tparam E - Type of the e.
     * @tparam Args - Type of the arguments.
     * @param args - Variable arguments providing [in,out] The event arguments.
     */
    template <typename E, typename... Args>
    void SendEvent(Args&&... args)
    {
        ECS_EventHandler->Send<E>(std::forward<Args>(event)...);
    }

    /**
     * Updates the entire ECS with a given delta time in milliseconds.
     * @param tickMS - The tick in milliseconds.
     */
    void Update(f32 tickMS);
};
} // namespace ecs
