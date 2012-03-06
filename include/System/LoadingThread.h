#ifndef BOMB_LOADING_THREAD_H
#define BOMB_LOADING_THREAD_H

#include <Global.h>
#include <Singleton.h>
#include <Spinlock.h>

enum LoadType
{
    LOAD_TEXTURE = 0,
    LOAD_MODEL   = 1,
    LOAD_MAX     = 2
};

struct LoadRecord
{
    LoadType type;
    uint32 sourceId;
    uint8 priority;
};

typedef std::pair<LoadType, uint32> LoadPair;

class LoaderWorker
{
    public:
        LoaderWorker();
        ~LoaderWorker();

        void RequestLoad(LoadType type, uint32 sourceId, uint8 prioritySign = 3);
        bool IsCurrentlyLoaded(LoadType type, uint32 sourceId);
        bool IsAlreadyLoaded(LoadType type, uint32 sourceId);
        void Worker();
    private:

        LoadPair m_currentlyLoaded;
        std::vector<LoadRecord> m_loadList;
};

#define sLoader Singleton<LoaderWorker>::instance()

extern void runLoaderWorker();

#endif
