#include "system.h"

namespace ecs
{

template <class T>
System<T>::System()
{
    DEFINE_LOGGER(typeid(T).name())
    LogInfo("System %s created.", typeid(T).name());
}

template <class T>
System<T>::~System()
{
    LogInfo("System %s released.", typeid(T).name());
}

template <class T>
template <class... Dependencies>
void System<T>::AddDependencies(Dependencies&&... dependencies)
{
    this->systemManager->AddSystemDependency(
        this, std::forward<Dependencies>(dependencies)...);

}

} // namespace ecs