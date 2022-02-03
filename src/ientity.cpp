#include "ientity.h"

namespace ecs
{
DEFINE_STATIC_LOGGER(IEntity, "Entity")

IEntity::IEntity()
    : m_Active(true)
{
}

IEntity::~IEntity() {}

void IEntity::SetActive(bool active)
{
    if (this->isActive == active)
        return;
    if (active)
    {
        this->OnEnable();
    }
    else
    {
        this->OnDisable();
    }
    this->isActive = active;
}

template <class T>
T* IEntity::GetComponent() const
{
    return this->m_ComponentManagerInstance->GetComponent<T>(this->entityId);
}

template <class T, class... P>
T* IEntity::AddComponent(P&&... param)
{
    return this->m_ComponentManagerInstance->AddComponent<T>(
        this->entityId, std::forward<P>(param)...);
}
template <class T>
void IEntity::RemoveComponent()
{
    this->m_ComponentManagerInstance->RemoveComponent<T>(this->entityId);
}
} // namespace ecs
