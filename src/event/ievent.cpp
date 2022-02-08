#include "event/ievent.h"

#include "engine.h"
#include "util/timer.h"

ecs::event::IEvent::IEvent(EventTypeId typeId)
    : typeId(typeId)
{
    assert(ecsEngine != nullptr && "ECS engine not initialized!");
    this->timeCreated = ecsEngine->ecsEngineTime->GetTimeStamp();
}
