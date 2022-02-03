#include "eventhandler.h"

ecs::event::EventHandler::EventHandler()
{
    this->GetEventMemoryAllocator() = new EventMemoryAllocator(
        ECS_EVENT_MEMORY_BUFFER_SIZE,
        Allocate(ECS_EVENT_MEMORY_BUFFER_SIZE, "EventHandler"));

    this->GetEventStorage().reserve(1024);
}

ecs::event::EventHandler::~EventHandler()
{
    for (EventHandler::EventDispatcherMap::iterator it =
             this->GetEventDispatcherMap().begin();
         it != this->GetEventDispatcherMap().end();
         ++it)
    {
        delete (*it).second;
        (*it).second = nullptr;
    }

    this->GetEventDispatcherMap().clear();

    // Release allocated memory
    Free((void*)this->GetEventMemoryAllocator()->GetMemoryAddress());

    delete this->GetEventMemoryAllocator();
    this->SetEventMemoryAllocator(nullptr);
}

void ecs::event::EventHandler::DispatchEvents()
{
    std::size_t lastIndex = this->GetEventStorage().size();
    std::size_t thisIndex = 0;

    while (thisIndex < lastIndex)
    {
        auto event = this->GetEventStorage()[thisIndex++];
        if (event == nullptr)
        {
            continue;
        }

        auto it = this->GetEventDispatcherMap().find(event->GetTypeID());
        if (it == this->GetEventDispatcherMap().end())
        {
            continue;
        }

        it->second->Dispatch(event);

        lastIndex = this->GetEventStorage().size();
    }

    this->ClearEventBuffer();
}
