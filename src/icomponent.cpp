#include "icomponent.h"

namespace ecs
{
IComponent::IComponent()
    : owner(INVALID_ENTITY_ID)
    , enabled(true)
{
}

IComponent::~IComponent() {}

} // namespace ecs