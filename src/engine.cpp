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
    ecsEngineTime        = new util::Timer();
    ecsEventHandler      = new event::EventHandler();
    ecsSystemManager     = new SystemManager();
    ecsComponentManager  = new ComponentManager();
    ecsEntityManager     = new EntityManager(this->ecsComponentManager);
}

EcsEngine::~EcsEngine()
{
    delete ecsEntityManager;
    ecsEntityManager = nullptr;

    delete ecsComponentManager;
    ecsComponentManager = nullptr;

    delete ecsSystemManager;
    ecsSystemManager = nullptr;

    delete ecsEventHandler;
    ecsEventHandler = nullptr;
}

void EcsEngine::Update(f32 tick_ms)
{
    // Advance engine time
    ecsEngineTime->Tick(tick_ms);

    // Update all running systems
    ecsSystemManager->Update(tick_ms);
    ecsEventHandler->DispatchEvents();

    // Finalize pending destroyed entities
    ecsEntityManager->RemoveDestroyedEntities();
    ecsEventHandler->DispatchEvents();
}

void EcsEngine::UnsubscribeEvent(event::internal::IEventDelegate* eventDelegate)
{
    ecsEventHandler->RemoveEventCallback(eventDelegate);
}

} // namespace ecs