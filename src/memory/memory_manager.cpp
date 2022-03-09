#include "memory/memory_manager.h"

ecs::memory::internal::MemoryManager::MemoryManager()
{
    DEFINE_LOGGER("MemoryManager")
    LogInfo("Initialize MemoryManager!");
    this->globalMemory = malloc(MemoryManager::MEMORY_CAPACITY);
    if (this->globalMemory != nullptr)
    {
    }
    else
    {
        assert(this->globalMemory != nullptr && "Failed to allocate global memory.");
    }

    this->memoryAllocator = new StackAllocator(MemoryManager::MEMORY_CAPACITY, this->globalMemory);
    assert(this->memoryAllocator != nullptr && "Failed to create memory allocator!");

    this->pendingMemory.clear();
    this->freedMemory.clear();
}

ecs::memory::internal::MemoryManager::~MemoryManager()
{
    this->memoryAllocator->Clear();
    delete this->memoryAllocator;
    this->memoryAllocator = nullptr;

    free(this->globalMemory);
    this->globalMemory = nullptr;
}

void* ecs::memory::internal::MemoryManager::Allocate(std::size_t memorySize, const std::string& user)
{
    LogDebug("%s allocated %d bytes of global memory.", user.data() != nullptr ? user.c_str() : "Unknown", memorySize);
    void* pointerMemory = memoryAllocator->Allocate(memorySize, alignof(u8));
    this->pendingMemory.push_back(std::make_pair(user, pointerMemory));
    return pointerMemory;
}

void ecs::memory::internal::MemoryManager::Free(void* pointerMemory)
{
    if (pointerMemory == this->pendingMemory.back().second)
    {
        this->memoryAllocator->Free(pointerMemory);
        this->pendingMemory.pop_back();

        bool check = true;
        while (check == true)
        {
            check = false;

            const auto& it =
                std::find_if(this->freedMemory.begin(),
                             this->freedMemory.end(),
                             [&](const void* pointer) { return pointer == this->pendingMemory.back().second; });

            if (it != this->freedMemory.end())
            {
                this->memoryAllocator->Free(pointerMemory);
                this->pendingMemory.pop_back();
                this->freedMemory.remove(*it);

                check = true;
            }
        }
    }
    else
    {
        this->freedMemory.push_back(pointerMemory);
    }
}

void ecs::memory::internal::MemoryManager::CheckMemoryLeaks()
{
    assert(!(this->freedMemory.size() > 0 && this->pendingMemory.size() == 0) && "Implementation failure");

    if (this->pendingMemory.size() > 0)
    {
        LogFatal("!!!  M E M O R Y   L E A K   D E T E C T E D  !!!")
            LogFatal("!!!  M E M O R Y   L E A K   D E T E C T E D  !!!")
                LogFatal("!!!  M E M O R Y   L E A K   D E T E C T E D  !!!")

                    for (const auto& i : this->pendingMemory)
        {
            bool isFreed = false;

            isFreed = std::any_of(this->freedMemory.begin(),
                                  this->freedMemory.end(),
                                  [&i](const void* pointer)
                                  {
                                      if (pointer == i.second)
                                      {
                                          return true;
                                      }
                                      return false;
                                  });

            if (isFreed == false)
            {
                LogFatal("\'%s\' memory user didn't release allocated memory %p!", i.first.c_str(), i.second)
            }
        }
    }
    else
    {
        LogInfo("No memory leaks detected.")
    }
}
