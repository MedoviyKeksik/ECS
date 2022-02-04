#pragma once

#include <list>

#include "ieventdelegate.h"
#include "ieventdispatcher.h"

namespace ecs
{
namespace event
{
namespace internal
{

template <typename T>
class EventDispatcher : public IEventDispatcher
{
    using EventDelegateList      = std::list<IEventDelegate*>;
    using PendingRemoveDelegates = std::list<IEventDelegate*>;

public:
    EventDispatcher();
    virtual ~EventDispatcher();

    virtual void Dispatch(IEvent* event) override;
    virtual void AddEventCallback(IEventDelegate* eventDelegate) override;
    virtual void RemoveEventCallback(IEventDelegate* eventDelegate) override;
    virtual std::size_t GetEventCallbackCount() const override;

private:
    inline void SetPendingRemoveDelegates(
        const PendingRemoveDelegates& pendingRemoveDelegates)
    {
        this->pendingRemoveDelegates = pendingRemoveDelegates;
    }
    inline auto& GetPendingRemoveDelegates()
    {
        return this->pendingRemoveDelegates;
    }
    inline const auto& GetPendingRemoveDelegates() const
    {
        return this->pendingRemoveDelegates;
    }

    inline void SetEventCallbacks(const EventDelegateList& eventCallbacks)
    {
        this->eventCallbacks = eventCallbacks;
    }
    inline auto&       GetEventCallbacks() { return this->eventCallbacks; }
    inline const auto& GetEventCallbacks() const
    {
        return this->eventCallbacks;
    }

    inline void        SetLocked(bool locked) { this->locked = locked; }
    inline auto&       GetLocked() { return this->locked; }
    inline const auto& GetLocked() const { return this->locked; }

private:
    PendingRemoveDelegates pendingRemoveDelegates;
    EventDelegateList      eventCallbacks;
    bool                   locked;
};

} // namespace internal
} // namespace event
} // namespace ecs
