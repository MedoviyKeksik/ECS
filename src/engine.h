#pragma once

#include <utility>

#include "./util/global.hpp"

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

class ECSEngine
{
    friend class IEntity;
    friend class IComponent;
    friend class ISystem;

    friend class event::IEvent;
    friend class event::IEventListener;

    friend class EntityManager;

public:
    ECSEngine();
    ~ECSEngine();

    inline EntityManager*    GetEntityManager() { return ECS_EntityManager; }
    inline ComponentManager* GetComponentManager()
    {
        return ECS_ComponentManager;
    }
    inline SystemManager* GetSystemManager() { return ECS_SystemManager; }

    template <typename E, typename... Args>
    void SendEvent(Args&&... args)
    {
        ECS_EventHandler->Send<E>(std::forward<Args>(args)...);
    }

    void Update(f32 tickMS);

private:
};
} // namespace ecs
