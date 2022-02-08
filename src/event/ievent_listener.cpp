#include "event/ievent_listener.h"

ecs::event::IEventListener::~IEventListener()
{
    this->UnregisterAllEventCallbacks();
}

void ecs::event::IEventListener::UnregisterAllEventCallbacks()
{
    for (auto cb : this->GetRegisteredCallbacks())
    {
        ecsEngine->UnsubscribeEvent(cb);
    }

    this->GetRegisteredCallbacks().clear();
}
