#include "icomponent.h"

namespace ecs
{
IComponent::IComponent()
    : m_Owner(INVALID_ENTITY_ID)
    , enabled(true)
{
}

IComponent::~IComponent() {}

} // namespace ecs