#pragma once

#include "ievent_delegate.h"

namespace ecs
{
namespace event
{

class IEvent;

namespace internal
{

class ECS_API IEventDispatcher
{
public:
    IEventDispatcher()          = default;
    virtual ~IEventDispatcher() = default;

    virtual void Dispatch(IEvent* evnt) = 0;

    virtual void AddEventCallback(IEventDelegate* const eventDelegate) = 0;

    virtual void RemoveEventCallback(IEventDelegate* eventDelegate) = 0;

    virtual std::size_t GetEventCallbackCount() const = 0;
};

} // namespace internal
} // namespace event
} // namespace ecs
