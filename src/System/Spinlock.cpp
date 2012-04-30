#include <Global.h>
#include <Spinlock.h>

/** \brief Konstruktor
 *
 * Vynulovani map uchazecu o token a holderu
 */
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

/** \brief Destruktor
 *
 * Prazdny
 */
SpinLock::~SpinLock()
{
}

/** \brief Funkce pro vyzadani tokenu o sdileny zdroj
 *
 * Pokud nikdo nema v drzeni token, priradi se automaticky hned
 */
void SpinLock::NeedToken(LockType target, LockThread applicant)
{
    ApplicantMap[target][applicant] = true;

    // Pokud nikdo nevyzadal token, pridelime si ho
    if (HolderMap[target] == THREAD_MAX)
        HolderMap[target] = applicant;
}

/** \brief Funkce pro vzdani se uchazecstvi o token sdileneho zdroje
 *
 * Pokud dane vlakno ma token, preda se dal
 */
void SpinLock::UnNeedToken(LockType target, LockThread releaser)
{
    ApplicantMap[target][releaser] = false;
    // Pokud mame token, predame ho
    // (funkce PassToken overi, zdali ho mame, dvojity check zbytecny)
    PassToken(target, releaser);
}

/** \brief Vraci true, pokud dany uchazec ma token k danemu sdilenemu zdroji
 */
bool SpinLock::HasToken(LockType target, LockThread questioner)
{
    if (HolderMap[target] == THREAD_MAX)
    {
        HolderMap[target] = questioner;
        return true;
    }

    return (HolderMap[target] == questioner);
}

/** \brief Funkce starajici se o predani tokenu nejakeho ze sdilenych zdroju
 *
 * Pokud se nikdo neuchazi o token, nastavi se dalsi applicant na THREAD_MAX, cili nejake makro neplatneho uchazece, a pote se pri vyzadani ihned prideli
 */
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
