#pragma once

#include <typeinfo>

#include "api.h"

#include "event/ievent_delegate.h"

namespace ecs
{
namespace event
{

class IEvent;

namespace internal
{

template <typename Class, typename EventType>
class ECS_API EventDelegate : public IEventDelegate
{
    typedef void (Class::*Callback)(const EventType* const);

public:
    EventDelegate(Class* receiver, Callback& callbackFunction)
        : receiver(receiver)
        , callback(callbackFunction)
    {
    }

    virtual IEventDelegate* clone() override { return new EventDelegate(this->receiver, this->callback); }

    virtual inline void invoke(const IEvent* const e) override
    {
        (receiver->*callback)(reinterpret_cast<const EventType* const>(e));
    }

    virtual inline EventDelegateId GetDelegateId() const override
    {
        static const EventDelegateId DELEGATE_ID{ typeid(Class).hash_code() ^ typeid(Callback).hash_code() };
        return DELEGATE_ID;
    }

    virtual inline u64 GetStaticEventTypeId() const override
    {
        static const u64 SEID{ EventType::STATIC_EVENT_TYPE_ID };
        return SEID;
    }

    virtual bool operator==(const IEventDelegate* other) const override
    {
        if (this->GetDelegateId() != other->GetDelegateId())
        {
            return false;
        }

        EventDelegate* delegate = (EventDelegate*)other;
        if (other == nullptr)
        {
            return false;
        }

        return ((this->callback == delegate->callback) && (this->receiver == delegate->receiver));
    }

private:
    inline void        SetReceiver(Class* receiver) { this->receiver = receiver; }
    inline auto&       GetReceiver() { return this->receiver; }
    inline const auto& GetReceiver() const { return this->receiver; }

    inline void        SetCallback(const Callback& callback) { this->callback = callback; }
    inline auto&       GetCallback() { return this->callback; }
    inline const auto& GetCallback() const { return this->callback; }

private:
    Class*   receiver;
    Callback callback;
};

} // namespace internal
} // namespace event
} // namespace ecs
