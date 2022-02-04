#include "ievent.h"

#include <cassert>

#include "../engine.h"

ecs::event::IEvent::IEvent(EventTypeId typeId)
    : typeId(typeId)
{
    int* ECS_Engine= nullptr;
    assert(ECS_Engine != nullptr && "ECS engine not initialized!");
      
    this->timeCreated = ECS_Engine->ECS_EngineTimeStamp();
}

ecs::event::IEvent::~IEvent() {}
