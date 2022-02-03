#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <unordered_map>
#include <utility>
#include <vector>

#include "../util/global.hpp"

#include "ievent.h"
#include "linearallocator.h"

namespace ecs
{
namespace event
{
class EventHandler : memory::GlobalMemoryUser
{
    friend class ECSEngine;

    using EventDispatcherMap =
        std::unordered_map<EventTypeId, internal::IEventDispatcher*>;

    using EventStorage         = std::vector<IEvent*>;
    using EventMemoryAllocator = memory::allocator::LinearAllocator;

public:
    EventHandler();
    ~EventHandler();

    inline void ClearEventBuffer()
    {
        this->GetEventMemoryAllocator()->Clear();
        this->GetEventStorage().clear();
    }

    inline void ClearEventDispatcher()
    {
        this->GetEventDispatcherMap().Clear();
    }

    template <typename E, typename... Args>
    void Send(Args&&... eventArgs)
    {
        static_assert(std::is_trivially_copyable<E>::value,
                      "Event is not trivially copuable.");

        void* pMem =
            this->GetEventMemoryAllocator()->Allocate(sizeof(E), alignof(E));

        if (pMem != nullptr)
        {
            this->GetEventStorage().push_back(
                new (pMem) E(std::forward<Args>(eventArgs)...));
        }
        else
        {
        }
    }

    void DispatchEvents()
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

            auto it ==
                this->GetEventDispatcherMap().find(event->GetEventTypeID());
            if (it == this->GetEventDispatcherMap().end())
            {
                continue;
            }

            it->second->Dispatch(event);

            lastIndex = this->GetEventStorage().size();
        }

        this->ClearEventBuffer();
    }

private:
    EventHandler(const EventHandler&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;

private:
    void SetEventDispatcherMap(const EventDispatcherMap& eventDispatcherMap)
    {
        this->eventDispatcherMap = eventDispatcherMap;
    }
    auto&       GetEventDispatcherMap() { return this->eventDispatcherMap; }
    const auto& GetEventDispatcherMap() const
    {
        return this->eventDispatcherMap;
    }

    void SetEventMemoryAllocator(EventMemoryAllocator* eventMemoryAllocator)
    {
        this->eventMemoryAllocator = eventMemoryAllocator;
    }
    auto       GetEventMemoryAllocator() { return this->eventMemoryAllocator; }
    const auto GetEventMemoryAllocator() const
    {
        return this->eventMemoryAllocator;
    }

    void SetEventStorage(const EventStorage& eventStorage)
    {
        this->eventStorage = eventStorage;
    }
    auto&       GetEventStorage() { return this->eventStorage; }
    const auto& GetEventStorage() const { return this->eventStorage; }

private:
    EventDispatcherMap    eventDispatcherMap;
    EventMemoryAllocator* eventMemoryAllocator;
    EventStorage          eventStorage;
};
} // namespace event

} // namespace ecs

#endif // EVENTHANDLER_H
