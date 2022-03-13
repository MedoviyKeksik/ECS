#include "engine.h"

#include "ecs.h"

#include "event/event_handler.h"

#include "util/timer.h"

namespace ecs
{

EcsEngine::EcsEngine()
{
    ecsEngineTime       = new util::Timer();
    ecsEventHandler     = new event::EventHandler();
    ecsSystemManager    = new SystemManager();
    ecsComponentManager = new ComponentManager();
    ecsEntityManager    = new EntityManager(this->ecsComponentManager);
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

    delete ecsEngineTime;
    ecsEngineTime = nullptr;
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
