#include <Global.h>
#include <Timer.h>

/** \brief Inicializace casovace - pouze smazani vsech udalosti
 */
void Timer::Initialize()
{
    TimedEvents.clear();
}

/** \brief Funkce zpracovavajici pozadavky o vlozeni zaznamu do pole casovace
 */
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

/** \brief Funkce zpracovavajici pozadavky na vlozeni zaznamu do pole casovace na nastaveni cile v pameti na danou hodnotu
 */
void Timer::AddTimedSetEvent(uint32 time, uint32 *target, uint32 value)
{
    TimerSetRecord temp;
    temp.expireTime = clock()+time;
    temp.target = target;
    temp.value  = value;
    TimedSetEvents.push_back(temp);
}

/** \brief Postara se o vymazani casovaneho zaznamu, ktery byl urcen adresou prvku v pameti
 */
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

/** \brief Hlavni update funkce casovace
 *
 * Zde se projdou vsechny zaznamy casovace a pokud doslo k jejich vyprseni, provede se jejich handler, pripadne se nastavi prvek identifikovany pozici v pameti na danou hodnotu
 */
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

/** \brief Funkce starajici se o pozastaveni casovacu
 */
void Timer::PauseTimers()
{
    m_pauseTime = clock();
}

/** \brief Funkce starajici se o odpauzovani casovacu
 */
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
