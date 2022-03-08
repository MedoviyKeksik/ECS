#pragma once

#include "api.h"

#include "event/event_delegate.h"
#include "event/event_handler.h"

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

public:
    EcsEngine();
    ~EcsEngine();

private:
    EcsEngine(const EcsEngine&) = delete;
    EcsEngine& operator=(EcsEngine&) = delete;

public:
    inline EntityManager* GetEntityManager() { return ecsEntityManager; }

    inline ComponentManager* GetComponentManager() { return ecsComponentManager; }
    inline SystemManager*    GetSystemManager() { return ecsSystemManager; }

    /**
     * Broadcasts an event.
     * @tparam E - Type of the e.
     * @tparam Args - Type of the arguments.
     * @param args - Variable arguments providing [in,out] The event arguments.
     */
    template <typename E, typename... Args>
    void SendEvent(Args&&... args)
    {
        ecsEventHandler->Send<E>(std::forward<Args>(args)...);
    }

    /**
     * Updates the entire ECS with a given delta time in milliseconds.
     * @param tickMS - The tick in milliseconds.
     */
    void Update(f32 tickMS);

private:
    // Add event callback
    template <class E>
    inline void SubscribeEvent(event::internal::IEventDelegate* const eventDelegate)
    {
        ecsEventHandler->AddEventCallback<E>(eventDelegate);
    }

    // Remove event callback
    void UnsubscribeEvent(event::internal::IEventDelegate* eventDelegate);

    util::Timer*         ecsEngineTime;
    EntityManager*       ecsEntityManager;
    ComponentManager*    ecsComponentManager;
    SystemManager*       ecsSystemManager;
    event::EventHandler* ecsEventHandler;
};
} // namespace ecs
