#include <Global.h>
#include <Spinlock.h>

SpinLock::SpinLock()
{
    ApplicantMap.resize(LOCK_MAX);
    for (uint32 i = 0; i < LOCK_MAX; i++)
    {
        ApplicantMap[i].resize(THREAD_MAX);

        for (uint32 j = 0; j < THREAD_MAX; j++)
            ApplicantMap[i][j] = false;
    }

    HolderMap.resize(LOCK_MAX);
    // Vynulovat tokeny
    for (uint32 i = 0; i < LOCK_MAX; i++)
        HolderMap[i] = THREAD_MAX;
}

SpinLock::~SpinLock()
{
}

void SpinLock::NeedToken(LockType target, LockThread applicant)
{
    ApplicantMap[target][applicant] = true;

    // Pokud nikdo nevyzadal token, pridelime si ho
    if (HolderMap[target] == THREAD_MAX)
        HolderMap[target] = applicant;
}

void SpinLock::UnNeedToken(LockType target, LockThread releaser)
{
    ApplicantMap[target][releaser] = false;
    // Pokud mame token, predame ho
    // (funkce PassToken overi, zdali ho mame, dvojity check zbytecny)
    PassToken(target, releaser);
}

bool SpinLock::HasToken(LockType target, LockThread questioner)
{
    return (HolderMap[target] == questioner);
}

void SpinLock::PassToken(LockType target, LockThread holder)
{
    // Kdyz token nemame, nemuzeme ho ani predat
    if (!HasToken(target, holder))
        return;

    LockThread next = THREAD_MAX;

    // projdeme zadatele od holdera dal
    for (uint32 i = holder+1; i < THREAD_MAX; i++)
    {
        if (ApplicantMap[target][i])
        {
            next = LockThread(i);
            break;
        }
    }

    // pokud jsme nenalezli...
    if (next == THREAD_MAX)
    {
        // ... projdeme jeste zadatele od nuly k holderovi
        for (uint32 i = 0; i <= holder; i++)
        {
            if (ApplicantMap[target][i])
            {
                next = LockThread(i);
                break;
            }
        }
    }

    // Neni nutne, aby token melo nejake z vlaken, kdyz ho nepotrebuje
    // Pokud nastane takova situace, holder tokenu se nastavi na hodnotu makra THREAD_MAX
    // pote se pri vyzadani tokenu take token automaticky prideli

    HolderMap[target] = next;
}
