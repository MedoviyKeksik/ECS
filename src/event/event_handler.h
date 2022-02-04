#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

#include "../util/global.hpp"

#include "ievent.h"
#include "linearallocator.h"

#include "eventdispatcher.h"

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

private:
    inline void SetEventDispatcherMap(
        const EventDispatcherMap& eventDispatcherMap)
    {
        this->eventDispatcherMap = eventDispatcherMap;
    }
    inline auto& GetEventDispatcherMap() { return this->eventDispatcherMap; }
    inline const auto& GetEventDispatcherMap() const
    {
        return this->eventDispatcherMap;
    }

    inline void SetEventMemoryAllocator(
        EventMemoryAllocator* eventMemoryAllocator)
    {
        this->eventMemoryAllocator = eventMemoryAllocator;
    }
    inline auto GetEventMemoryAllocator() { return this->eventMemoryAllocator; }
    inline const auto GetEventMemoryAllocator() const
    {
        return this->eventMemoryAllocator;
    }

    inline void SetEventStorage(const EventStorage& eventStorage)
    {
        this->eventStorage = eventStorage;
    }
    inline auto&       GetEventStorage() { return this->eventStorage; }
    inline const auto& GetEventStorage() const { return this->eventStorage; }

public:
    inline void ClearEventBuffer()
    {
        this->GetEventMemoryAllocator()->Clear();
        this->GetEventStorage().clear();
    }

    inline void ClearEventDispatcher()
    {
        this->GetEventDispatcherMap().clear();
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

    void DispatchEvents();

private:
    EventHandler(const EventHandler&) = delete;
    EventHandler& operator=(const EventHandler&) = delete;

private:
    EventDispatcherMap    eventDispatcherMap;
    EventMemoryAllocator* eventMemoryAllocator;
    EventStorage          eventStorage;
};
} // namespace event

} // namespace ecs
