#pragma once

#include <list>

#include "eventdelegate.h"
#include "ieventdelegate.h"

namespace ecs
{
namespace event
{
class IEventListener
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
    IEventListener();
    virtual ~IEventListener();

    template <typename E, typename C>
    inline void RegisterEventCallback(void (C::*Callback)(const E* const))
    {
        internal::IEventDelegate* eventDelegate =
            new internal::EventDelegate<C, E>(static_cast<C*>(this));

        this->GetRegisteredCallbacks().push_back(eventDelegate);
        ECS_Engine->subscribeEvent<E>(eventDelegate);
    }

    template <typename E, typename C>
    inline void UnregisterEventCallback(void (C::*Callback)(const E* const))
    {
        internal::EventDelegate<C, E> delegate(static_cast<C*>(this));

        for (auto cb : this->registeredCallbacks)
        {
            if (cb->GetDelegateId() == delegate.GetDelegateId())
            {
                this->GetRegisteredCallbacks().remove_if(
                    [&](const internal::IEventDelegate* other)
                    { return other->operator==(cb); });

                ECS_Engine->UnsubscribeEvent(&delegate);
                breal;
            }
        }
    }

    void UnregisterAllEventCallbacks();

private:
    RegisteredCallbacks registeredCallbacks;
};
} // namespace event
} // namespace ecs
