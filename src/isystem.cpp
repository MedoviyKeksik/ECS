#include "isystem.h"

namespace ecs
{

ISystem::ISystem(SystemPriority priority, f32 updateInterval_ms)
    : systemPriority(priority)
    , updateInterval(updateInterval_ms)
    , isEnabled(true)
{
}

ISystem::~ISystem() {}

} // namespace ecs