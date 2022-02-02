#include "icomponent.h"

namespace ecs
{
IComponent::IComponent()
    : m_Owner(INVALID_ENTITY_ID)
    , m_Enabled(true)
{
}

IComponent::~IComponent() {}

} // namespace ecs