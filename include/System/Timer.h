#ifndef __TIMER_H__INCL__
#define __TIMER_H__INCL__

#include <Global.h>
#include <Singleton.h>

struct TimerRecord
{
    uint32 maxtime; //milisekundy
    uint32 remaintime; //milisekundy
    uint32 param1;
    uint32 param2;
    uint32 param3;
    void (*Handler)(uint32 param1, uint32 param2, uint32 param3);
};

class Timer
{
public:
    Timer() {};
    ~Timer() {};

    void Initialize();
    void Update(uint32 diff);

    void AddTimedEvent(uint32 time, void (*Handler)(uint32, uint32, uint32),uint32 param1, uint32 param2, uint32 param3);

protected:
    std::list<TimerRecord> TimedEvents;
};

#define sTimer Singleton<Timer>::instance()

#endif
