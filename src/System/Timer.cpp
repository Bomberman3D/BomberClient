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
    temp.expireTime = clock()+time;
    temp.param1 = param1;
    temp.param2 = param2;
    temp.param3 = param3;
    TimedEvents.push_back(temp);
}

void Timer::AddTimedSetEvent(uint32 time, uint32 *target, uint32 value)
{
    TimerSetRecord temp;
    temp.expireTime = clock()+time;
    temp.target = target;
    temp.value  = value;
    TimedSetEvents.push_back(temp);
}

void Timer::RemoveTimerSetEventByTarget(uint32* target)
{
    // THREAD UNSAFE !

    if (TimedSetEvents.empty())
        return;

    for (std::list<TimerSetRecord>::iterator itr = TimedSetEvents.begin(); itr != TimedSetEvents.end();)
    {
        if (itr->target == target)
            itr = TimedSetEvents.erase(itr);
        else
            ++itr;
    }
}

void Timer::Update()
{
    if (m_pauseTime > 0)
        return;

    clock_t tnow = clock();

    if (!TimedEvents.empty())
    {
        for (std::list<TimerRecord>::iterator itr = TimedEvents.begin(); itr != TimedEvents.end();)
        {
            if (itr->expireTime <= tnow)
            {
                if (itr->Handler != NULL)
                    itr->Handler(itr->param1, itr->param2, itr->param3);
                itr = TimedEvents.erase(itr);
            }
            else
                ++itr;
        }
    }

    if (!TimedSetEvents.empty())
    {
        for (std::list<TimerSetRecord>::iterator itr = TimedSetEvents.begin(); itr != TimedSetEvents.end();)
        {
            if (itr->expireTime <= tnow)
            {
                if (itr->target != NULL)
                    (*(itr->target)) = itr->value;
                itr = TimedSetEvents.erase(itr);
            }
            else
                ++itr;
        }
    }
}

void Timer::PauseTimers()
{
    m_pauseTime = clock();
}

void Timer::UnpauseTimers()
{
    clock_t diff = clock() - m_pauseTime;

    m_pauseTime = 0;

    if (!TimedEvents.empty())
    {
        for (std::list<TimerRecord>::iterator itr = TimedEvents.begin(); itr != TimedEvents.end();++itr)
            itr->expireTime += diff;
    }

    if (!TimedSetEvents.empty())
    {
        for (std::list<TimerSetRecord>::iterator itr = TimedSetEvents.begin(); itr != TimedSetEvents.end();++itr)
            itr->expireTime += diff;
    }
}
