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

/** \var TimerRecord::expireTime
 *  \brief Cas, ve kterem dojde ke spusteni udalosti casovace
 */

/** \var TimerRecord::param1
 *  \brief Parametr 1 udalosti casovace
 */

/** \var TimerRecord::param2
 *  \brief Parametr 2 udalosti casovace
 */

/** \var TimerRecord::param3
 *  \brief Parametr 3 udalosti casovace
 */

/** \var TimerRecord::Handler(uint32 param1, uint32 param2, uint32 param3)
 *  \brief Ukazatel na funkci, ktera se spusti po ubehnuti daneho casu
 */

struct TimerSetRecord
{
    clock_t expireTime;
    uint32* target;
    uint32  value;
};

/** \var TimerSetRecord::expireTime
 *  \brief Cas, ve kterejm dojde k nastaveni targetu na danou hodnotu
 */

/** \var TimerSetRecord::target
 *  \brief Ukazatel na cilovou pamet, ktera ma byt nasavena po ubehnuti casu
 */

/** \var TimerSetRecord::value
 *  \brief Hodnota, na kterou se target nastavi
 */

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

        void PauseTimers();
        void UnpauseTimers();

    protected:
        std::list<TimerRecord> TimedEvents;
        std::list<TimerSetRecord> TimedSetEvents;

        clock_t m_pauseTime;
};

#define sTimer Singleton<Timer>::instance()

/** \var Timer::TimedEvents
 *  \brief Pole casovanych udalosti
 */

/** \var Timer::TimedSetEvents
 *  \brief Pole casovanych nastaveni prvku identifikovanymi adresou
 */

/** \var Timer::m_pauseTime
 *  \brief Cas, kdy byly timery pozastaveny (a zaroven priznak pozastaveni)
 */

#endif
