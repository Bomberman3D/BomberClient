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

/** \struct LoadRecord
 *  \brief Zaznam v poli zdroju k nacteni
 */
struct LoadRecord
{
    LoadType type;
    uint32 sourceId;
    uint8 priority;
};

/** \var LoadRecord::type
 *  \brief Typ zaznamu, viz enumerator LoadType
 */

/** \var LoadRecord::sourceId
 *  \brief ID zdrojoveho zaznamu k nacteni (model / textura)
 */

/** \var LoadRecord::priority
 *  \brief Priorita nacitaneho zdroje
 */

typedef std::pair<LoadType, uint32> LoadPair;

/** \class LoaderWorker
 *  \brief Hlavni nacitaci trida, starajici se o nacteni zdroju a spousteni / zastavovani nacitaciho vlakna
 */
class LoaderWorker
{
    public:
        LoaderWorker();
        ~LoaderWorker();

        void RequestLoad(LoadType type, uint32 sourceId, uint8 prioritySign = 3);
        void RequestLoadBlocking(LoadType type, uint32 sourceId);
        bool IsCurrentlyLoaded(LoadType type, uint32 sourceId);
        bool IsAlreadyLoaded(LoadType type, uint32 sourceId);
        void Worker();

        void ShutdownThread();
        bool m_isDead;
    private:

        LoadPair m_currentlyLoaded;
        std::vector<LoadRecord> m_loadList;
        bool m_isShuttingDown;
        bool m_somethingToLoad;
};

#define sLoader Singleton<LoaderWorker>::instance()

extern void runLoaderWorker();

#endif
