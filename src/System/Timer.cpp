#include <Global.h>
#include <Timer.h>

void Timer::Initialize()
{
    TimedEvents.clear();
}

void Timer::AddTimedEvent(uint32 time, void (*Handler)(uint32, uint32, uint32), uint32 param1, uint32 param2, uint32 param3)
{
    TimerRecord temp;
    temp.Handler = Handler;
    temp.maxtime = time;
    temp.remaintime = time;
    temp.param1 = param1;
    temp.param2 = param2;
    temp.param3 = param3;
    TimedEvents.push_back(temp);
}

void Timer::Update(uint32 diff)
{
    if (TimedEvents.empty())
        return;

    for (std::list<TimerRecord>::iterator itr = TimedEvents.begin(); itr != TimedEvents.end();)
    {
        if (itr->remaintime <= diff)
        {
            itr->Handler(itr->param1, itr->param2, itr->param3);
            itr = TimedEvents.erase(itr);
        }
        else
        {
            itr->remaintime -= diff;
            ++itr;
        }
    }
}
