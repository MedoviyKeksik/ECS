#include "engine.h"

#include "eventhandler.h"

#include "componentmanager.h"
#include "entitymanager.h"
#include "systemmanager.h"

#include "timer.h"

namespace ecs
{

EcsEngine::EcsEngine()
{
    ECS_EngineTime       = new util::Timer();
    ECS_EventHandler     = new event::EventHandler();
    ECS_SystemManager    = new SystemManager();
    ECS_ComponentManager = new ComponentManager();
    ECS_EntityManager    = new EntityManager(this->ECS_ComponentManager);
}

EcsEngine::~EcsEngine()
{
    delete ECS_EntityManager;
    ECS_EntityManager = nullptr;

    delete ECS_ComponentManager;
    ECS_ComponentManager = nullptr;

    delete ECS_SystemManager;
    ECS_SystemManager = nullptr;

    delete ECS_EventHandler;
    ECS_EventHandler = nullptr;
}

void EcsEngine::Update(f32 tick_ms)
{
    // Advance engine time
    ECS_EngineTime->Tick(tick_ms);

    // Update all running systems
    ECS_SystemManager->Update(tick_ms);
    ECS_EventHandler->DispatchEvents();

    // Finalize pending destroyed entities
    ECS_EntityManager->RemoveDestroyedEntities();
    ECS_EventHandler->DispatchEvents();
}

void EcsEngine::UnsubscribeEvent(event::internal::IEventDelegate* eventDelegate)
{
    ECS_EventHandler->RemoveEventCallback(eventDelegate);
}

} // namespace ecs