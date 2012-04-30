#ifndef BOMB_SPINLOCK_H
#define BOMB_SPINLOCK_H

#include <Global.h>
#include <Singleton.h>

enum LockType
{
    LOCK_DISPLAYLIST = 0,
    LOCK_LOADLIST,
    LOCK_STORAGE,
    LOCK_MAX
};

enum LockThread
{
    THREAD_MAIN = 0,
    THREAD_NETWORK,
    THREAD_ANIMATION,
    THREAD_LOADING,
    THREAD_MAX
};

/** \class SpinLock
 *  \brief Manager spinlockoveho zamku sdilenych ulozist
 */
class SpinLock
{
    public:
        SpinLock();
        ~SpinLock();

        void NeedToken(LockType target, LockThread applicant);
        void UnNeedToken(LockType target, LockThread releaser);
        bool HasToken(LockType target, LockThread questioner);
        void PassToken(LockType target, LockThread holder);

    private:
        vector<vector<bool>> ApplicantMap;
        vector<LockThread>   HolderMap;
};

#define sLockMgr Singleton<SpinLock>::instance()

#endif
