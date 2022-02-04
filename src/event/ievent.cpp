#include "ievent.h"

#include <cassert>

ecs::event::IEvent::IEvent(EventTypeId typeId)
    : m_typeId(typeId)
{
    int* ECS_Engine= nullptr;
    assert(ECS_Engine != nullptr && "ECS engine not initialized!");
    //this->m_timeCreated = ECS_Engine->ECS_EngineTimeStamp();
}
