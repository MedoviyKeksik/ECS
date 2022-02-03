#pragma once

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
} // namespace internal
} // namespace event
} // namespace ecs
