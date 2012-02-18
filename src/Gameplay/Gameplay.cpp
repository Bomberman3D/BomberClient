#include <Global.h>
#include <Gameplay.h>
#include <Timer.h>
#include <Map.h>
#include <Effects/Animations.h>
#include <Effects/ParticleEmitter.h>

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

    if (!DangerousMap.empty() && !m_gamePaused)
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
    m_gamePaused = false;

    m_moveElements.resize(MOVE_MAX);
    for (uint8 i = 0; i < MOVE_MAX; i++)
        m_moveElements[i] = false;

    m_playerRec = sDisplay->DrawModel(9, 0.5f, 0, 0.5f, ANIM_IDLE, 3.5f, 90.0f, true, false, 0, 0, ANIM_RESTRICTION_NOT_PAUSED);
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

    BombRecord* bomb = new BombRecord;
    bomb->x = x;
    bomb->y = y;
    bomb->state = 0;
    bomb->reach = GetFlameReach();
    BombMap.push_back(bomb);

    sTimer->AddTimedSetEvent(2500, &bomb->state, 1);
    m_plActiveBombs++;

    if (sGameplayMgr->GetGameType() == GAME_TYPE_SP_CLASSIC)
        localPlayerStats.ClassicSingleStats.bombsPlanted += 1;

    Map* pMap = (Map*)sMapManager->GetMap();
    if (pMap)
    {
        // Nyni promenliva hodnota podle bonusu
        // Bonus se ale zapocita jen pokud je bomba polozena az po vzeti bonusu, ne na vsechny
        // Jednak je to realistictejsi a jednak to usnadni trochu pathfinding pro AI
        uint32 bombreach = bomb->reach;

        // Projdeme vsechny mozne zaznamy na mape az na maximalni definovany dosah
        // pokud tam neco je, zamezime dalsimu prepsani a zapiseme
        uint32 reach_x1 = bombreach+1, reach_x2 = bombreach+1, reach_y1 = bombreach+1, reach_y2 = bombreach+1;
        // Promenna urcujici, zdali doslo ke kolizi kvuli objektu (true) nebo kvuli dosahu (false)
        bool rx1_box = false, rx2_box = false, ry1_box = false, ry2_box = false;

        for (uint32 i = 0; i < bombreach; i++)
        {
            // Pokud jeste nebylo zapsano
            if (reach_x1 == bombreach+1)
            {
                // Overime, zdali na dane pozici neco neprostupneho je
                if (pMap->IsDynamicRecordPresent(bomb->x + i + 1, bomb->y, DYNAMIC_TYPE_BOX))
                {
                    // Pokud ano, zamezime zapsanim maximalni pozice dalsimu prepsani
                    reach_x1 = i+1;
                    rx1_box = true;
                }
                else if (pMap->GetStaticRecord(bomb->x + i + 1,bomb->y) == TYPE_SOLID_BOX)
                    reach_x1 = i+1;
            }

            if (reach_x2 == bombreach+1)
            {
                if (pMap->IsDynamicRecordPresent(bomb->x - i - 1, bomb->y, DYNAMIC_TYPE_BOX))
                {
                    reach_x2 = i+1;
                    rx2_box = true;
                }
                else if (pMap->GetStaticRecord(bomb->x - i - 1,bomb->y) == TYPE_SOLID_BOX)
                    reach_x2 = i+1;
            }

            if (reach_y1 == bombreach+1)
            {
                if (pMap->IsDynamicRecordPresent(bomb->x, bomb->y + i + 1, DYNAMIC_TYPE_BOX))
                {
                    reach_y1 = i+1;
                    ry1_box = true;
                }
                else if (pMap->GetStaticRecord(bomb->x,bomb->y + i + 1) == TYPE_SOLID_BOX)
                    reach_y1 = i+1;
            }

            if (reach_y2 == bombreach+1)
            {
                if (pMap->IsDynamicRecordPresent(bomb->x, bomb->y - i - 1, DYNAMIC_TYPE_BOX))
                {
                    reach_y2 = i+1;
                    ry2_box = true;
                }
                else if (pMap->GetStaticRecord(bomb->x,bomb->y - i - 1) == TYPE_SOLID_BOX)
                    reach_y2 = i+1;
            }
        }
        // Pak vsechno dekrementujeme - zapise se vzdy pozice uz nepristupneho pole
        --reach_x1;
        --reach_x2;
        --reach_y1;
        --reach_y2;

        // Nakonec nacasujeme vybuch na vsech "nebezpecnych" polich, aby se bedny odpalily s dobre casovanym vybuchem
        // (v pripade true hodnoty bool promenne +1 / -1 proto, ze reach je definovan pro dosah plamene. Bedna je o jedno pole dal)
        clock_t tnow = clock();

        sGameplayMgr->SetDangerous(bomb->x, bomb->y, tnow+2500, 1500);
        for (uint32 i = 1; i <= bombreach; i++)
        {
            if (reach_x1 + (rx1_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x + i, bomb->y, tnow+i*100+2500, 1500);
            if (reach_x2 + (rx2_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x - i, bomb->y, tnow+i*100+2500, 1500);
            if (reach_y1 + (ry1_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x, bomb->y + i, tnow+i*100+2500, 1500);
            if (reach_y2 + (ry2_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x, bomb->y - i, tnow+i*100+2500, 1500);
        }
    }

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

bool GameplayMgr::WouldBeDangerousField(uint32 x, uint32 y)
{
    std::map<std::pair<uint32, uint32>, DangerousField*>::const_iterator itr = DangerousMap.find(std::make_pair(x,y));
    if (itr == DangerousMap.end())
        return false;

    return true;
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

void GameplayMgr::PauseGame()
{
    if (IsGamePaused())
        return;

    m_gamePaused = true;
    m_pauseTime = clock();

    if (IsSingleGameType())
    {
        BlockMovement();
        sTimer->PauseTimers();
        sParticleEmitterMgr->PauseEmitters();
        sDisplay->DisableRestrictedAnimations(ANIM_RESTRICTION_NOT_PAUSED);
    }
}

void GameplayMgr::UnpauseGame()
{
    if (!IsGamePaused())
        return;

    m_gamePaused = false;
    clock_t diff = clock() - m_pauseTime;
    m_pauseTime = 0;

    if (IsSingleGameType())
    {
        UnblockMovement();
        sTimer->UnpauseTimers();
        sParticleEmitterMgr->UnpauseEmitters();
        sDisplay->EnableRestrictedAnimations(ANIM_RESTRICTION_NOT_PAUSED);
    }

    if (!DangerousMap.empty())
    {
        for (std::map<std::pair<uint32, uint32>, DangerousField*>::iterator itr = DangerousMap.begin(); itr != DangerousMap.end(); ++itr)
        {
            if ((*itr).second->registered)
                (*itr).second->activeTime += diff;
            else
                (*itr).second->activeSince += diff;
        }
    }

    int middleX = sConfig->WindowWidth >> 1;
    int middleY = sConfig->WindowHeight >> 1;
    SetCursorPos(middleX, middleY);
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
            sAnimator->ChangeModelAnim(m_playerRec->AnimTicket, ANIM_WALK, 25, 2);

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
