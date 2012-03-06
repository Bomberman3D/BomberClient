#include <Global.h>
#include <LoadingThread.h>
#include <Storage.h>
#include <ImageLoader.h>
#include <ModelLoader.h>

LoaderWorker::LoaderWorker()
{
    m_loadList.clear();
    m_currentlyLoaded = std::make_pair(LOAD_MAX, 0);
}

LoaderWorker::~LoaderWorker()
{
}

bool LoaderWorker::IsCurrentlyLoaded(LoadType type, uint32 sourceId)
{
    if (m_currentlyLoaded.first == type && m_currentlyLoaded.second == sourceId)
        return true;

    return false;
}

bool LoaderWorker::IsAlreadyLoaded(LoadType type, uint32 sourceId)
{
    if (type == LOAD_TEXTURE && sStorage->Textures[sourceId] != NULL)
        return true;

    if (type == LOAD_MODEL && sStorage->Models[sourceId] != NULL)
        return true;

    return false;
}

void LoaderWorker::RequestLoad(LoadType type, uint32 sourceId, uint8 prioritySign)
{
    // Nejdrive overime, jestli uz neni dany prvek nacten, nebo zrovna nacitan
    if (IsAlreadyLoaded(type, sourceId) || IsCurrentlyLoaded(type, sourceId))
        return;

    // Sem budeme vstupovat jen a pouze z hlavniho vlakna aplikace - proto si musime zamknout loadlist pro hlavni vlakno
    sLockMgr->NeedToken(LOCK_LOADLIST, THREAD_MAIN);

    while (!sLockMgr->HasToken(LOCK_LOADLIST, THREAD_MAIN))
    {
    }

    // Projdeme list, jestli uz nahodou nekdo nezadal o nacteni daneho prvku
    if (!m_loadList.empty())
    {
        for (std::vector<LoadRecord>::iterator itr = m_loadList.begin(); itr != m_loadList.end(); ++itr)
        {
            // Pokud ano, porovname prioritu, jestli nahodou neexistuji prvky s mensi prioritou (vetsim indexem priority)
            // a pripadne jim prioritu navysime podle vstupniho argumentu
            if ((*itr).type == type && (*itr).sourceId == sourceId)
            {
                if ((*itr).priority > prioritySign)
                    (*itr).priority = prioritySign;

                return;
            }
        }
    }

    // Ted muzeme pridat zaznam do naseho seznamu pro nacteni
    uint32 index = m_loadList.size(); // novy index bude budouci "velikost-1", proto si muzeme vzit to cislo hned ted - pred pridanim

    m_loadList.resize(m_loadList.size()+1);
    m_loadList[index].type = type;
    m_loadList[index].sourceId = sourceId;
    m_loadList[index].priority = prioritySign;

    sLockMgr->UnNeedToken(LOCK_LOADLIST, THREAD_MAIN);
}

void LoaderWorker::Worker()
{
    LoadPair chosen = std::make_pair(LOAD_MAX, 0);

    while(1)
    {
        sLockMgr->NeedToken(LOCK_LOADLIST, THREAD_LOADING);

        while (!sLockMgr->HasToken(LOCK_LOADLIST, THREAD_LOADING))
        {
        }

        // operace s loadlistem
        if (!m_loadList.empty())
        {
            // Podle priority
            for (uint32 i = 0; i <= 3; i++)
            {
                for (std::vector<LoadRecord>::iterator itr = m_loadList.begin(); itr != m_loadList.end(); ++itr)
                {
                    // Bereme v potaz jen priority 0, 1 a 2, dalsi uz se neberou nejak vyrazne v potaz
                    if ((i == 3 || (*itr).priority == i) && !IsAlreadyLoaded((*itr).type, (*itr).sourceId))
                    {
                        chosen = std::make_pair((*itr).type, (*itr).sourceId);
                        m_loadList.erase(itr);
                        break;
                    }
                }

                if (chosen.first != LOAD_MAX)
                    break;
            }
        }

        m_currentlyLoaded = chosen;

        sLockMgr->UnNeedToken(LOCK_LOADLIST, THREAD_LOADING);

        if (chosen.first == LOAD_TEXTURE)
            Loaders::LoadTexture(chosen.second);
        else if (chosen.first == LOAD_MODEL)
            Loaders::LoadModel(chosen.second);

        chosen = std::make_pair(LOAD_MAX, 0);
        m_currentlyLoaded = chosen;
    }
}

void runLoaderWorker()
{
    sLoader->Worker();
}