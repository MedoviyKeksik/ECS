#include "isystem.h"

namespace ecs
{

ISystem::ISystem(SystemPriority priority, f32 updateInterval_ms)
    : m_Priority(priority)
    , m_UpdateInterval(updateInterval_ms)
    , m_Enabled(true)
{
}

ISystem::~ISystem() {}

} // namespace ecs