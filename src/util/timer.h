
#ifndef ECS_TIMER_H
#define ECS_TIMER_H

#include "../api.h"

namespace ecs::util
{

class Timer
{
    using Elapsed = std::chrono::duration<f32, std::milli>;

public:
    Timer()  = default;
    ~Timer() = default;

    void Tick(f32 ms){};

    void Reset(){};

    [[nodiscard]] inline TimeStamp GetTimeStamp() const
    {
        return TimeStamp(this->m_Elapsed.count());
    }

private:
    Elapsed m_Elapsed;

}; // class Timer

} // namespace ecs::util

#endif // ECS_TIMER_H
