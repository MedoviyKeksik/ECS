#include "ievent.h"

#include "../engine.h"
#include "../util/timer.h"

ecs::event::IEvent::IEvent(EventTypeId typeId)
    : typeId(typeId)
{
    assert(Ecs_Engine != nullptr && "ECS engine not initialized!");
    this->timeCreated = Ecs_Engine->ecsEngineTime->GetTimeStamp();
}
