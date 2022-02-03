#ifndef IEVENTDELEGATE_H
#define IEVENTDELEGATE_H

#include <typeinfo>

#include "../util/global.hpp"

namespace ecs
{
namespace event
{
class IEvent;

namespace internal
{
using EventDelegateId = std::size_t;

class IEventDelegate
{
public:
    IEventDelegate()          = default;
    virtual ~IEventDelegate() = default;

    virtual inline void            invoke(const IEvent* const e)          = 0;
    virtual inline EventDelegateId GetDelegateId() const                  = 0;
    virtual inline u64             GetStaticEventTypeId() const           = 0;
    virtual bool            operator==(const IEventDelegate* other) const = 0;
    virtual IEventDelegate* clone()                                       = 0;

}; //  IEventDelegate;

template <typename Class, typename EventType>
class EventDelegate : public IEventDelegate
{
    typedef void (Class::*Callback)(const EventType* const);

    Class*   receiver;
    Callback callback;

public:
    EventDelegate(Class* receiver, Callback& callbackFunction)
        : receiver(receiver)
        , callback(callbackFunction)
    {
    }

    virtual ~IEventDelegate() {}

    virtual IEventDelegate* clone() override
    {
        return new EventDelegate(this->receiver, this->callback);
    }

    virtual inline void invoke(const IEvent* const e) override
    {
        (receiver->*callback)(reinterpret_cast<const EventType* const>(e));
    }

    virtual inline EventDelegate GetDelegateId() const override
    {
        static const EventDelegateId DELEGATE_ID{
            typeid(Class).hash_code() ^ typeid(Callback).hash_code()
        };
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

        return ((this->callback == delegate->callback) &&
                (this->receiver == delegate->receiver));
    }
};

} // namespace internal
} // namespace event
} // namespace ecs

#endif // IEVENTDELEGATE_H
