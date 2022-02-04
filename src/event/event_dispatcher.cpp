#include "eventdispatcher.h"

#include <algorithm>
#include <cassert>

template <typename T>
ecs::event::internal::EventDispatcher<T>::EventDispatcher()
    : locked(false)
{
}

template <typename T>
ecs::event::internal::EventDispatcher<T>::~EventDispatcher()
{
    this->GetPendingRemoveDelegates().clear();
    this->GetEventCallbacks().clear();
}

template <typename T>
void ecs::event::internal::EventDispatcher<T>::Dispatch(IEvent* event)
{
    this->SetLocked(true);
    if (this->GetPendingRemoveDelegates().empty == false)
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

template <typename T>
void ecs::event::internal::EventDispatcher<T>::AddEventCallback(
    IEventDelegate* eventDelegate)
{
    auto result = std::find_if(this->GetPendingRemoveDelegates().begin(),
                               this->GetPendingRemoveDelegates().end(),
                               [&](const IEventDelegate* other)
                               { return other->operator==(eventDelegate); });

    if (result != this->GetPendingRemoveDelegates().end())
    {
        this->GetPendingRemoveDelegates().erase(result);
        return;
    }

    this->GetEventCallbacks().push_back(eventDelegate);
}

template <typename T>
void ecs::event::internal::EventDispatcher<T>::RemoveEventCallback(
    IEventDelegate* eventDelegate)
{
    if (this->GetLocked() == false)
    {
        auto result = std::find_if(this->GetEventCallbacks().begin(),
                                   this->GetEventCallbacks().end(),
                                   [&](const IEventDelegate* other) {
                                       return other->operator==(eventDelegate);
                                   });

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
        auto result = std::find_if(this->GetEventCallbacks().begin(),
                                   this->GetEventCallbacks().end(),
                                   [&](const IEventDelegate* other) {
                                       return other->operator==(eventDelegate);
                                   });
        //        assert(result != this->GetEventCallbacks().end() && "");
        if (result != this->GetEventCallbacks().end())
        {
            this->GetPendingRemoveDelegates().push_back((*result));
        }
    }
}

template <typename T>
std::size_t ecs::event::internal::EventDispatcher<T>::GetEventCallbackCount()
    const
{
    return this->GetEventCallbacks().size();
}
