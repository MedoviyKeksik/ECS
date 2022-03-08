#pragma once

#include <list>

#include "api.h"

#include "event/ievent_dispatcher.h"
#include "log/logger_macro.h"

namespace ecs
{
namespace event
{
namespace internal
{

template <typename T>
class ECS_API EventDispatcher : public IEventDispatcher
{
    DECLARE_STATIC_LOGGER

    using EventDelegateList      = std::list<IEventDelegate*>;
    using PendingRemoveDelegates = std::list<IEventDelegate*>;

public:
    EventDispatcher()
        : locked(false)
    {
    }
    virtual ~EventDispatcher()
    {
        this->GetPendingRemoveDelegates().clear();
        this->GetEventCallbacks().clear();
    }

    virtual void Dispatch(IEvent* event) override
    {
        this->SetLocked(true);
        LogTrace("Dispatch event %s", typeid(T).name());
        if (this->GetPendingRemoveDelegates().empty() == false)
        {
            for (auto EC : this->GetPendingRemoveDelegates())
            {
                auto result = std::find_if(this->GetEventCallbacks().begin(),
                                           this->GetEventCallbacks().end(),
                                           [&](const IEventDelegate* other)
                                           { return other->operator==(EC); });
                if (result != this->GetEventCallbacks().end())
                {
                    IEventDelegate* ptrMem = (IEventDelegate*)(*result);
                    this->GetEventCallbacks().erase(result);
                    delete ptrMem;
                    ptrMem = nullptr;
                }
            }
            this->GetPendingRemoveDelegates().clear();
        }

        for (auto EC : this->GetEventCallbacks())
        {
            assert(EC != nullptr && "Invalid event callback.");
            EC->invoke(event);
        }

        this->SetLocked(false);
    }
    virtual void AddEventCallback(IEventDelegate* eventDelegate) override
    {
        auto result = std::find_if(this->GetPendingRemoveDelegates().begin(),
                                   this->GetPendingRemoveDelegates().end(),
                                   [&](const IEventDelegate* other) {
                                       return other->operator==(eventDelegate);
                                   });

        if (result != this->GetPendingRemoveDelegates().end())
        {
            this->GetPendingRemoveDelegates().erase(result);
            return;
        }

        this->GetEventCallbacks().push_back(eventDelegate);
    }
    virtual void RemoveEventCallback(IEventDelegate* eventDelegate) override
    {
        if (this->GetLocked() == false)
        {
            auto result =
                std::find_if(this->GetEventCallbacks().begin(),
                             this->GetEventCallbacks().end(),
                             [&](const IEventDelegate* other)
                             { return other->operator==(eventDelegate); });

            if (result != this->GetEventCallbacks().end())
            {
                IEventDelegate* ptrMem = (IEventDelegate*)(*result);

                this->GetEventCallbacks().erase(result);

                delete ptrMem;
                ptrMem = nullptr;
            }
        }
        else
        {
            auto result =
                std::find_if(this->GetEventCallbacks().begin(),
                             this->GetEventCallbacks().end(),
                             [&](const IEventDelegate* other)
                             { return other->operator==(eventDelegate); });
            //        assert(result != this->GetEventCallbacks().end() && "");
            if (result != this->GetEventCallbacks().end())
            {
                this->GetPendingRemoveDelegates().push_back((*result));
            }
        }
    }
    virtual std::size_t GetEventCallbackCount() const override
    {
        return this->GetEventCallbacks().size();
    }

private:
    inline void SetPendingRemoveDelegates(
        const PendingRemoveDelegates& pendingRemoveDelegates)
    {
        this->pendingRemoveDelegates = pendingRemoveDelegates;
    }
    inline auto& GetPendingRemoveDelegates()
    {
        return this->pendingRemoveDelegates;
    }
    inline const auto& GetPendingRemoveDelegates() const
    {
        return this->pendingRemoveDelegates;
    }

    inline void SetEventCallbacks(const EventDelegateList& eventCallbacks)
    {
        this->eventCallbacks = eventCallbacks;
    }
    inline auto&       GetEventCallbacks() { return this->eventCallbacks; }
    inline const auto& GetEventCallbacks() const
    {
        return this->eventCallbacks;
    }

    inline void        SetLocked(bool locked) { this->locked = locked; }
    inline auto&       GetLocked() { return this->locked; }
    inline const auto& GetLocked() const { return this->locked; }

private:
    PendingRemoveDelegates pendingRemoveDelegates;
    EventDelegateList      eventCallbacks;
    bool                   locked;
};

DEFINE_STATIC_LOGGER_TEMPLATE(EventDispatcher, T, "EventDispatcher")

} // namespace internal
} // namespace event
} // namespace ecs
