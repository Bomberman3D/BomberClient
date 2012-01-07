#ifndef __TIMER_H__INCL__
#define __TIMER_H__INCL__

#include <Global.h>
#include <Singleton.h>

struct TimerRecord
{
    clock_t expireTime;
    uint32 param1;
    uint32 param2;
    uint32 param3;
    void (*Handler)(uint32 param1, uint32 param2, uint32 param3);
};

struct TimerSetRecord
{
    clock_t expireTime;
    uint32* target;
    uint32  value;
};

class Timer
{
public:
    Timer() {};
    ~Timer() {};

    void Initialize();
    void Update();

    void AddTimedEvent(uint32 time, void (*Handler)(uint32, uint32, uint32),uint32 param1, uint32 param2, uint32 param3);
    void AddTimedSetEvent(uint32 time, uint32* target, uint32 value);
    void RemoveTimerSetEventByTarget(uint32* target);

protected:
    std::list<TimerRecord> TimedEvents;
    std::list<TimerSetRecord> TimedSetEvents;
};

#define sTimer Singleton<Timer>::instance()

#endif
