#ifndef ECS_SYSTEM_H
#define ECS_SYSTEM_H

#include "api.h"

#include "familytypeid.h"
#include "isystem.h"

namespace ecs
{
template <class T>
class System : public ISystem
{
    friend class SystemManager;

protected:
    DECLARE_LOGGER
    System();

public:
    virtual ~System();

    static const SystemTypeId STATIC_SYSTEM_TYPE_ID;

    virtual inline const SystemTypeId GetStaticSystemTypeID() const
    {
        return STATIC_SYSTEM_TYPE_ID;
    }

    virtual inline const char* GetSystemTypeName() const override
    {
        static const char* SYSTEM_TYPE_NAME{ typeid(T).name() };
        return SYSTEM_TYPE_NAME;
    }

    /**
     * Adds a new dependencies for this system.
     * @tparam Dependencies - Type of the dependencie
     * @param dependencies - Variable arguments providing [in,out] The
     * dependencies.
     */
    template <class... Dependencies>
    void AddDependencies(Dependencies&&... dependencies);

    virtual void PreUpdate(f32 dt) override {}

    virtual void Update(f32 dt) override {}

    virtual void PostUpdate(f32 dt) override {}

private:
    SystemManager* systemManager;

}; // class System<T>

template <class T>
const SystemTypeId System<T>::STATIC_SYSTEM_TYPE_ID =
    util::internal::FamilyTypeID<ISystem>::Get<T>();

} // namespace ecs

#endif // ECS_SYSTEM_H
