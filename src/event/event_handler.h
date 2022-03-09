#pragma once

#include "api.h"

#include "memory/allocators/linear_allocator.h"

#include "event/event_dispatcher.h"
#include "event/ievent.h"

namespace ecs
{
namespace event
{
class ECS_API EventHandler : memory::GlobalMemoryUser
{
    friend class ecs::EcsEngine;

    DECLARE_LOGGER

    using EventDispatcherMap = std::unordered_map<EventTypeId, internal::IEventDispatcher*>;

    using EventStorage         = std::vector<IEvent*>;
    using EventMemoryAllocator = memory::allocator::LinearAllocator;

public:
    EventHandler();
    ~EventHandler();

private:
    inline void SetEventDispatcherMap(const EventDispatcherMap& eventDispatcherMap)
    {
        this->eventDispatcherMap = eventDispatcherMap;
    }
    inline auto&       GetEventDispatcherMap() { return this->eventDispatcherMap; }
    inline const auto& GetEventDispatcherMap() const { return this->eventDispatcherMap; }

    inline void SetEventMemoryAllocator(EventMemoryAllocator* eventMemoryAllocator)
    {
        this->eventMemoryAllocator = eventMemoryAllocator;
    }
    inline auto       GetEventMemoryAllocator() { return this->eventMemoryAllocator; }
    inline const auto GetEventMemoryAllocator() const { return this->eventMemoryAllocator; }

    inline void        SetEventStorage(const EventStorage& eventStorage) { this->eventStorage = eventStorage; }
    inline auto&       GetEventStorage() { return this->eventStorage; }
    inline const auto& GetEventStorage() const { return this->eventStorage; }

public:
    inline void ClearEventBuffer()
    {
        this->GetEventMemoryAllocator()->Clear();
        this->GetEventStorage().clear();
    }

    inline void ClearEventDispatcher() { this->GetEventDispatcherMap().clear(); }

    template <typename E, typename... Args>
    void Send(Args&&... eventArgs)
    {
        //        static_assert(std::is_trivially_copyable<E>::value,
        //                      "Event is not trivially copyable.");

        void* pMem = this->GetEventMemoryAllocator()->Allocate(sizeof(E), alignof(E));

        if (pMem != nullptr)
        {
            this->GetEventStorage().push_back(new (pMem) E(std::forward<Args>(eventArgs)...));
            LogTrace("\'%s\' event buffered.", typeid(E).name());
        }
        else
        {
            LogWarning("Event buffer is full! Cut off new incoming events !!!");
        }
    }

    void DispatchEvents();

private:
    EventHandler(const EventHandler&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;

private:
    // Add event callback
    template <class E>
    inline void AddEventCallback(internal::IEventDelegate* const eventDelegate)
    {
        EventTypeId ETID = E::STATIC_EVENT_TYPE_ID;

        EventDispatcherMap::const_iterator iter = this->GetEventDispatcherMap().find(ETID);
        if (iter == this->GetEventDispatcherMap().end())
        {
            std::pair<EventTypeId, internal::IEventDispatcher*> kvp(ETID, new internal::EventDispatcher<E>());

            kvp.second->AddEventCallback(eventDelegate);

            this->GetEventDispatcherMap().insert(kvp);
        }
        else
        {
            this->GetEventDispatcherMap()[ETID]->AddEventCallback(eventDelegate);
        }
    }

    // Remove event callback
    inline void RemoveEventCallback(internal::IEventDelegate* eventDelegate)
    {
        auto                               typeId = eventDelegate->GetStaticEventTypeId();
        EventDispatcherMap::const_iterator iter   = this->GetEventDispatcherMap().find(typeId);
        if (iter != this->GetEventDispatcherMap().end())
        {
            this->GetEventDispatcherMap()[typeId]->RemoveEventCallback(eventDelegate);
        }
    }

private:
    EventDispatcherMap    eventDispatcherMap;
    EventMemoryAllocator* eventMemoryAllocator;
    EventStorage          eventStorage;
};
} // namespace event

} // namespace ecs
