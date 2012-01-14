#include <Global.h>
#include <Gameplay.h>
#include <Timer.h>
#include <Map.h>
#include <Effects/Animations.h>

GameplayMgr::GameplayMgr()
{
    m_game = NULL;

    m_settings.resize(SETTING_MAX);
    m_settings[SETTING_ENEMY_COUNT] = 4;
    m_settings[SETTING_ENEMY_AI_LEVEL] = 1;
    m_settings[SETTING_ENEMY_SPEED] = 1;
    m_settings[SETTING_BOX_DENSITY] = 10;
    m_settings[SETTING_MAP_ID] = 1;

    memset(&localPlayerStats, 0, sizeof(PlayerStats::UniversalStatTemplate));
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

void GameplayMgr::OnGameInit()
{
    // Vychozi hodnoty
    m_plSpeedCoef  = 1.0f;
    m_plFlameReach = 1;
    m_plMaxBombs   = 1;

    m_plActiveBombs = 0;
    m_movementBlocked = false;

    m_moveElements.resize(MOVE_MAX);
    for (uint8 i = 0; i < MOVE_MAX; i++)
        m_moveElements[i] = false;

    m_playerRec = sDisplay->DrawModel(1, 0.5f, 0, 0.5f, ANIM_IDLE, 0.20f, 90.0f, true);
    m_moveAngle = 0.0f;
    m_playerX = 0;
    m_playerY = 0;
    m_playerRec->rotate = m_moveAngle;

    sDisplay->SetTargetModel(m_playerRec);

    memset(&localPlayerStats, 0, sizeof(PlayerStats::UniversalStatTemplate));

    if (!BombMap.empty())
    {
        for (std::list<BombRecord*>::iterator itr = BombMap.begin(); itr != BombMap.end();)
        {
            delete (*itr);
            itr = BombMap.erase(itr);
        }

        BombMap.clear();
    }

    if (!DangerousMap.empty())
    {
        for (std::map<std::pair<uint32, uint32>, DangerousField*>::iterator itr = DangerousMap.begin(); itr != DangerousMap.end();)
        {
            delete (*itr).second;
            itr = DangerousMap.erase(itr);
        }

        DangerousMap.clear();
    }

    if (m_game)
        m_game->OnGameInit(m_playerRec);
}

void GameplayMgr::OnGameLeave()
{
    if (m_game)
        m_game->OnGameLeave();
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

    if (sGameplayMgr->GetGameType() == GAME_TYPE_SP_CLASSIC)
        localPlayerStats.ClassicSingleStats.bombsPlanted += 1;

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

void GameplayMgr::PlayerDied(uint32 x, uint32 y)
{
    BlockMovement();

    if (sAnimator->GetAnimId(m_playerRec->AnimTicket) == ANIM_WALK)
        sAnimator->ChangeModelAnim(m_playerRec->AnimTicket, ANIM_IDLE, 0, 0);

    sApplication->SetStagePhase(3);
}

void GameplayMgr::UpdatePlayerMoveAngle()
{
    if (m_movementBlocked)
        return;

    // Otoceni hrace (hracskeho modelu) o uhel prepocitany podle pohybu mysi
    // Pri FPS rezimu musime nastavit i vertikalni uhel, ale to ve funkci Display::AdjustViewToTarget
    POINT mousePos;
    int middleX = sConfig->WindowWidth >> 1;
    int middleY = sConfig->WindowHeight >> 1;
    GetCursorPos(&mousePos);
    SetCursorPos(middleX, middleY);  // Posuneme mys zase na stred
    if (!((mousePos.x == middleX) && (mousePos.y == middleY)))
        m_moveAngle += 0.0025f*(mousePos.x-middleX);
}

void GameplayMgr::UpdatePlayerMotion(uint32 diff)
{
    if (m_movementBlocked)
        return;

    // Za jednu milisekundu musime urazit 0.002 jednotky, tzn. 1s = 2 jednotky
    float dist = (float(diff)+1.0f)*0.002f*GetPlayerSpeedCoef();
    float angle_rad = m_moveAngle;

    bool move = false;

    if (m_moveElements[MOVE_FORWARD])
    {
        move = true;

        if (m_moveElements[MOVE_LEFT])
            angle_rad -= PI/4;
        if (m_moveElements[MOVE_RIGHT])
            angle_rad += PI/4;
    }
    else
    {
        if (m_moveElements[MOVE_BACKWARD])
        {
            move = true;
            angle_rad -= PI;

            if (m_moveElements[MOVE_LEFT])
                angle_rad += PI/4;
            if (m_moveElements[MOVE_RIGHT])
                angle_rad -= PI/4;
        }
        else
        {
            if (m_moveElements[MOVE_LEFT])
            {
                move = true;
                angle_rad -= PI/2;
            }
            if (m_moveElements[MOVE_RIGHT])
            {
                move = true;
                angle_rad += PI/2;
            }
        }
    }

    m_playerRec->rotate = (PI/2-angle_rad)*180.0f / PI;
    sDisplay->DeviateHorizontalAngle(- (angle_rad - m_moveAngle)*180.0f/PI);

    if (move)
    {
        if (sAnimator->GetAnimId(m_playerRec->AnimTicket) != ANIM_WALK)
            sAnimator->ChangeModelAnim(m_playerRec->AnimTicket, ANIM_WALK, 0, 5);

        // Nejdrive se zkontroluje kolize na ose X
        float newx = m_playerRec->x + dist*cos(angle_rad);
        float newz = m_playerRec->z;
        uint16 collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        // Pokud na tehle ose nekolidujeme, muzeme se posunout
        if (!(collision & AXIS_X))
            m_playerRec->x = newx;

        // Nasleduje posun po ose Z
        newx = m_playerRec->x;
        newz = m_playerRec->z + dist*sin(angle_rad);
        collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        // A opet pokud nekolidujeme na dane ose, posuneme hrace
        if (!(collision & AXIS_Z))
            m_playerRec->z = newz;
    }
    else
    {
        if (sAnimator->GetAnimId(m_playerRec->AnimTicket) == ANIM_WALK)
            sAnimator->ChangeModelAnim(m_playerRec->AnimTicket, ANIM_IDLE, 0, 0);
    }

    // A nakonec vsechno prelozime tak, aby se pohled zarovnal k hraci
    // Nutne pro spravne zobrazeni
    sDisplay->AdjustViewToTarget();

    // Pripadne obslouzeni vstupu na jine pole - univerzalni check pro treba vstup do plamenu, na bonus, ...
    uint32 nX = ceil(m_playerRec->x);
    uint32 nY = ceil(m_playerRec->z);
    if (nX != m_playerX || nY != m_playerY)
    {
        OnPlayerFieldChange(m_playerX, m_playerY, nX, nY);
        m_playerX = nX;
        m_playerY = nY;
    }
}
