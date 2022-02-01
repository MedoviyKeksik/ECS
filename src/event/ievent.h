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

    inline const EventTypeId GetEventTypeID() const { return this->m_typeId; }
    inline const EventTimeStamp GetTimeCreated() const
    {
        return this->m_timeCreated;
    }

private:
    EventTypeId    m_typeId;
    EventTimeStamp m_timeCreated;
};

} // namespace event
} // namespace ecs
