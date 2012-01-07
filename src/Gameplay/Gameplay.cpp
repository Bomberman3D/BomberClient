#include <Global.h>
#include <Gameplay.h>
#include <Timer.h>
#include <Map.h>

GameplayMgr::GameplayMgr()
{
    m_game = NULL;

    m_settings.resize(SETTING_MAX);
    m_settings[SETTING_ENEMY_COUNT] = 4;
    m_settings[SETTING_ENEMY_AI_LEVEL] = 1;
    m_settings[SETTING_ENEMY_SPEED] = 1;
    m_settings[SETTING_BOX_DENSITY] = 10;
    m_settings[SETTING_MAP_ID] = 1;
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

    clock_t tnow = clock();

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
                m_game->OnBombBoom(*itr);
                m_plActiveBombs--;
                itr = BombMap.erase(itr);
                continue;
            }

            ++itr;
        }
    }

    if (!DangerousMap.empty())
    {
        for (std::map<std::pair<uint32, uint32>, DangerousField*>::iterator itr = DangerousMap.begin(); itr != DangerousMap.end();)
        {
            if (!itr->second)
            {
                ++itr;
                continue;
            }

            if (itr->second->activeSince + itr->second->activeTime < tnow)
            {
                itr = DangerousMap.erase(itr);
                continue;
            }

            if (itr->second->registered)
            {
                ++itr;
                continue;
            }

            if (itr->second->activeSince <= tnow)
            {
                itr->second->registered = true;
                if (m_game)
                    m_game->OnDangerousFieldActivate(itr->first.first, itr->first.second);
            }

            ++itr;
        }
    }
}

void GameplayMgr::OnGameInit(ModelDisplayListRecord* pPlayerRec)
{
    // Vychozi hodnoty
    m_plSpeedCoef  = 1.0f;
    m_plFlameReach = 1;
    m_plMaxBombs   = 1;

    m_plActiveBombs = 0;

    if (m_game)
        m_game->OnGameInit(pPlayerRec);
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

bool GameplayMgr::AddBomb(uint32 x, uint32 y)
{
    if (m_plActiveBombs >= m_plMaxBombs)
        return false;

    BombRecord* temp = new BombRecord;
    temp->x = x;
    temp->y = y;
    temp->state = 0;
    temp->reach = GetFlameReach();
    BombMap.push_back(temp);

    sTimer->AddTimedSetEvent(2500, &temp->state, 1);
    m_plActiveBombs++;
    return true;
}

void GameplayMgr::OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY)
{
    if (m_game)
        m_game->OnPlayerFieldChange(oldX, oldY, newX, newY);
}

bool GameplayMgr::IsDangerousField(uint32 x, uint32 y)
{
    std::map<std::pair<uint32, uint32>, DangerousField*>::const_iterator itr = DangerousMap.find(std::make_pair(x,y));
    if (itr == DangerousMap.end())
        return false;

    if (itr->second->activeSince <= clock() && itr->second->activeSince+itr->second->activeTime >= clock())
        return true;

    return false;
}

void GameplayMgr::SetDangerous(uint32 x, uint32 y, clock_t since, uint32 howLong)
{
    DangerousField* field = new DangerousField;

    field->activeSince = since;
    field->activeTime = howLong;
    field->registered = false;

    DangerousMap[std::make_pair(x,y)] = field;
}
