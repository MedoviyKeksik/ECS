#pragma once

#include "../util/global.hpp"

namespace ecs
{
namespace event
{
using EventTypeId    = TypeID;
using EventTimeStamp = TimeStamp;

static const EventTypeId INVALID_EVENT_TYPE = INVALID_TYPE_ID;

class IEvent
{
public:
    IEvent(EventTypeId typeId);

    inline const EventTypeId    GetEventTypeID() const { return this->typeId; }
    inline const EventTimeStamp GetTimeCreated() const
    {
        return this->timeCreated;
    }

private:
    EventTypeId    typeId;
    EventTimeStamp timeCreated;
};

} // namespace event
} // namespace ecs
