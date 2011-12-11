#include <Global.h>
#include <Gameplay.h>
#include <Timer.h>
#include <Map.h>

GameplayMgr::GameplayMgr()
{
    m_game = NULL;
}

GameplayMgr::~GameplayMgr()
{
    if (m_game)
        delete m_game;
}

void GameplayMgr::Update()
{
    if (m_game)
        m_game->OnUpdate();

    if (!BombMap.empty())
    {
        for (std::list<BombRecord*>::iterator itr = BombMap.begin(); itr != BombMap.end();)
        {
            if (!(*itr))
            {
                ++itr;
                continue;
            }

            if ((*itr)->state == 1)
            {
                Map* pMap = (Map*)sMapManager->GetMap();
                if (pMap)
                {
                    pMap->DestroyDynamicRecords((*itr)->x, (*itr)->y, DYNAMIC_TYPE_BOMB);
                    itr = BombMap.erase(itr);
                    continue;
                }
            }

            ++itr;
        }
    }
}

void GameplayMgr::OnGameInit()
{
    if (m_game)
        m_game->OnGameInit();
}

void GameplayMgr::SetGameType(GameType type)
{
    if (type == GAME_TYPE_MAX)
        return;

    if (m_game)
        delete m_game;

    switch (type)
    {
        case GAME_TYPE_NONE:
            m_game = new GameTypeTemplate;
            break;
        case GAME_TYPE_SP_CLASSIC:
            m_game = new ClassicSingleGameType;
            break;
        case GAME_TYPE_MP_CLASSIC:
            m_game = new ClassicMultiGameType;
            break;
        default:
            break;
    }

    assert(m_game != NULL);
}

GameType GameplayMgr::GetGameType()
{
    if (!m_game)
        return GAME_TYPE_NONE;

    return m_game->GetType();
}

void GameplayMgr::AddBomb(uint32 x, uint32 y)
{
    BombRecord* temp = new BombRecord;
    temp->x = x;
    temp->y = y;
    temp->state = 0;
    BombMap.push_back(temp);

    sTimer->AddTimedSetEvent(2500, &temp->state, 1);
}
