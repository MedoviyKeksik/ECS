#pragma once

#include "api.h"
#include "engine.h"

#include "event/event_delegate.h"

namespace ecs
{
namespace event
{
class ECS_API IEventListener
{
    using RegisteredCallbacks = std::list<internal::IEventDelegate*>;

private:
    inline void SetRegisteredCallbacks(
        const RegisteredCallbacks& registeredCallbacks)
    {
        this->registeredCallbacks = registeredCallbacks;
    }
    inline auto& GetRegisteredCallbacks() { return this->registeredCallbacks; }
    inline const auto& GetRegisteredCallbacks() const
    {
        return this->registeredCallbacks;
    }

public:
    IEventListener() = default;
    virtual ~IEventListener();

    template <typename E, typename C>
    inline void RegisterEventCallback(void (C::*Callback)(const E* const))
    {
        internal::IEventDelegate* eventDelegate =
            new internal::EventDelegate<C, E>(static_cast<C*>(this), Callback);

        this->GetRegisteredCallbacks().push_back(eventDelegate);
        ecsEngine->SubscribeEvent<E>(eventDelegate);
    }

    template <typename E, typename C>
    inline void UnregisterEventCallback(void (C::*Callback)(const E* const))
    {
        internal::EventDelegate<C, E> delegate(static_cast<C*>(this), Callback);

        for (auto cb : this->registeredCallbacks)
        {
            if (cb->GetDelegateId() == delegate.GetDelegateId())
            {
                this->GetRegisteredCallbacks().remove_if(
                    [&](const internal::IEventDelegate* other)
                    { return other->operator==(cb); });

                ecsEngine->UnsubscribeEvent(&delegate);
                break;
            }
        }
    }

    void UnregisterAllEventCallbacks();

private:
    RegisteredCallbacks registeredCallbacks;
};
} // namespace event
} // namespace ecs
