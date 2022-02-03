#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

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
    void SetPendingRemoveDelegates(
        const PendingRemoveDelegates& pendingRemoveDelegates)
    {
        this->pendingRemoveDelegates = pendingRemoveDelegates;
    }
    auto& GetPendingRemoveDelegates() { return this->pendingRemoveDelegates; }
    const auto& GetPendingRemoveDelegates() const
    {
        return this->pendingRemoveDelegates;
    }

    void SetEventCallbacks(const EventDelegateList& eventCallbacks)
    {
        this->eventCallbacks = eventCallbacks;
    }
    auto&       GetEventCallbacks() { return this->eventCallbacks; }
    const auto& GetEventCallbacks() const { return this->eventCallbacks; }

    void        SetLocked(bool locked) { this->locked = locked; }
    auto&       GetLocked() { return this->locked; }
    const auto& GetLocked() const { return this->locked; }

private:
    PendingRemoveDelegates pendingRemoveDelegates;
    EventDelegateList      eventCallbacks;
    bool                   locked;
};

} // namespace internal
} // namespace event
} // namespace ecs

#endif // EVENTDISPATCHER_H
