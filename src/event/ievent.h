#pragma once

#include "api.h"

namespace ecs
{
namespace event
{
using EventTypeId    = TypeID;
using EventTimeStamp = TimeStamp;

static const EventTypeId INVALID_EVENT_TYPE = INVALID_TYPE_ID;

class ECS_API IEvent
{
public:
    IEvent(EventTypeId typeId);
    virtual ~IEvent() = default;

    inline const auto& GetTypeID() const { return this->typeId; }
    inline auto&       GetTypeID() { return this->typeId; }
    inline const auto& GetTimeCreated() const { return this->timeCreated; }
    inline auto&       GetTimeCreated() { return this->timeCreated; }

private:
    inline void SetTypeID(const EventTypeId eventTypeID)
    {
        this->typeId = eventTypeID;
    }

    inline void SetTimeCreated(const EventTimeStamp timeCreated)
    {
        this->timeCreated = timeCreated;
    }

private:
    EventTypeId    typeId;
    EventTimeStamp timeCreated;
};

} // namespace event
} // namespace ecs
