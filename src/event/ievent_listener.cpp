#include "ievenelistener.h"

ecs::event::IEventListener::IEventListener() {}

ecs::event::IEventListener::~IEventListener()
{
    this->UnregisterAllEventCallbacks();
}

void ecs::event::IEventListener::UnregisterAllEventCallbacks()
{
    for (auto cb : this->GetRegisteredCallbacks())
    {
        ECS_Engine->UnsubscribeEvent(cb);
    }

    this->GetRegisteredCallbacks().clear();
}
