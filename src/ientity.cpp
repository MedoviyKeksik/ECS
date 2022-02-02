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
    if (this->m_Active == active)
        return;
    if (active)
    {
        this->OnEnable();
    }
    else
    {
        this->OnDisable();
    }
    this->m_Active = active;
}

template <class T>
T* IEntity::GetComponent() const
{
    return this->m_ComponentManagerInstance->GetComponent<T>(this->m_EntityID);
}

template <class T, class... P>
T* IEntity::AddComponent(P&&... param)
{
    return this->m_ComponentManagerInstance->AddComponent<T>(
        this->m_EntityID, std::forward<P>(param)...);
}
template <class T>
void IEntity::RemoveComponent()
{
    this->m_ComponentManagerInstance->RemoveComponent<T>(this->m_EntityID);
}
} // namespace ecs
