#include <Global.h>
#include <Gameplay.h>
#include <Timer.h>
#include <Map.h>
#include <Effects/Animations.h>
#include <Effects/ParticleEmitter.h>
#include <SoundMgr.h>
#include <Network.h>

/** \brief Konstruktor
 *
 * Jen a pouze nulovani
 */
GameplayMgr::GameplayMgr()
{
    m_game = NULL;
    m_lastMovementUpdate = 0;
    m_gameEndTime = 0;
    m_lastMoveHeartbeat = 0;
    m_stepSoundIndicator = 0;
    m_lastUpdatedMoveAngle = 0.0f;
    m_forceHeartbeatSend = false;

    m_settings.resize(SETTING_MAX);
    m_settings[SETTING_ENEMY_COUNT] = 4;
    m_settings[SETTING_ENEMY_AI_LEVEL] = 1;
    m_settings[SETTING_ENEMY_SPEED] = 1;
    m_settings[SETTING_BOX_DENSITY] = 10;
    m_settings[SETTING_MAP_ID] = 1;

    memset(&localPlayerStats, 0, sizeof(PlayerStats::UniversalStatTemplate));
}

/** \brief Destruktor
 *
 * Pouze uvolneni aktivni hry
 */
GameplayMgr::~GameplayMgr()
{
    if (m_game)
        delete m_game;
}

/** \brief Hlavni update funkce
 *
 * Volana z tridy herni faze
 */
void GameplayMgr::Update()
{
    if (m_game)
        m_game->OnUpdate();

    // Zvuky kroku
    if (m_playerRec)
    {
        if (m_playerRec->modelId == 9 || m_playerRec->modelId == 11)
        {
            if (sAnimator->GetActualFrame(m_playerRec->AnimTicket) >= 50 && m_stepSoundIndicator == 0)
            {
                sSoundMgr->PlayEffect(18 + rand()%4);
                m_stepSoundIndicator = 1;
            }
            else if (sAnimator->GetActualFrame(m_playerRec->AnimTicket) < 50 && m_stepSoundIndicator == 1)
            {
                sSoundMgr->PlayEffect(18 + rand()%4);
                m_stepSoundIndicator = 0;
            }
        }
    }

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

            if ((*itr)->boomTime <= tnow)
            {
                sSoundMgr->StopSoundEffect((*itr)->sizzleSound);
                sSoundMgr->PlayEffect(1);

                // Nahodna hlaska
                // Do budoucna osetrit singleplayer(pouze lokalni hrac mluvi) / multiplayer(muzou i ostatni!)
                if (rand()%5 == 0)
                {
                    if (!GetGameTypeResources()->RandomSayIDs.empty())
                        sSoundMgr->PlayEffect(GetGameTypeResources()->RandomSayIDs[rand()%(GetGameTypeResources()->RandomSayIDs.size())], false, false, 1000);
                }

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
        DangerousField* temp = NULL;

        for (uint32 i = 0; i < DangerousMap.size(); i++)
        {
            for (uint32 j = 0; j < DangerousMap[0].size(); j++)
            {
                for (uint32 k = 0; k < 4; k++)
                {
                    if (!DangerousMap[i][j][k])
                        continue;

                    temp = DangerousMap[i][j][k];

                    if (temp->activeSince + temp->activeTime < tnow)
                    {
                        delete DangerousMap[i][j][k];
                        DangerousMap[i][j][k] = NULL;
                        continue;
                    }

                    if (temp->registered)
                        continue;

                    if (temp->activeSince <= tnow)
                    {
                        temp->registered = true;
                        if (m_game)
                            m_game->OnDangerousFieldActivate(i, j);

                        PreBoomBomb(i, j);
                    }
                }
            }
        }
    }
}

/** \brief Inicializace hry a hernich principu
 *
 * Nulovani vychozich hodnot, vytvoreni zaznamu hrace, nulovani map bomb a nebezpecnych poli
 */
void GameplayMgr::OnGameInit()
{
    // Vychozi hodnoty
    m_plSpeedCoef  = 1.0f;
    m_plFlameReach = 1;
    m_plMaxBombs   = 1;

    m_plActiveBombs = 0;
    m_movementBlocked = false;
    m_playerDead = false;
    m_gamePaused = false;
    m_gameEndTime = 0;

    m_moveElements.resize(MOVE_MAX);
    for (uint8 i = 0; i < MOVE_MAX; i++)
        m_moveElements[i] = false;

    m_lastMovementUpdate = clock();

    m_playerRec = m_game->SpawnNewPlayer();
    if (!m_playerRec)
        m_playerRec = sDisplay->DrawModel(9, 0.5f, 0, 0.5f, ANIM_IDLE, 3.5f, 90.0f, true, false, 0, 0, ANIM_RESTRICTION_NOT_PAUSED);

    m_moveAngle = 0.0f;
    m_playerX = 0;
    m_playerY = 0;
    m_playerRec->rotate = m_moveAngle;
    sDisplay->SetAngleX(40.0f);

    sDisplay->SetTargetModel(m_playerRec);

    memset(&localPlayerStats, 0, sizeof(PlayerStats::UniversalStatTemplate));

    m_console = false;
    memset(&m_cheatMap, 0, sizeof(m_cheatMap));
    for (uint32 i = 0; i < CONSOLE_OUTPUT_LINES; i++)
        m_consoleOutput[i] = "";

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
        for (std::vector<std::vector<std::vector<DangerousField*>>>::iterator iter = DangerousMap.begin(); iter != DangerousMap.end();)
        {
            for (std::vector<std::vector<DangerousField*>>::iterator itr = (*iter).begin(); itr != (*iter).end();)
            {
                for (uint32 i = 0; i < 4; i++)
                {
                    if ((*itr)[i] != NULL)
                        delete (*itr)[i];
                }
                (*itr).clear();
                itr = (*iter).erase(itr);
            }
            iter = DangerousMap.erase(iter);
        }

        DangerousMap.clear();
    }

    Map* pMap = (Map*)sMapManager->GetMap();
    if (pMap)
    {
        DangerousMap.clear();
        DangerousMap.resize(pMap->field.size());
        for (uint32 i = 0; i < DangerousMap.size(); i++)
        {
            DangerousMap[i].clear();
            DangerousMap[i].resize(pMap->field[i].size());
            for (uint32 j = 0; j < DangerousMap[i].size(); j++)
            {
                DangerousMap[i][j].clear();
                DangerousMap[i][j].resize(4);
                for (uint32 k = 0; k < 4; k++)
                    DangerousMap[i][j][k] = NULL;
            }
        }
    }

    if (m_game)
        m_game->OnGameInit(m_playerRec);
}

/** \brief Funkce slouzici k zavolani stejnojmenne metody herniho typu
 */
void GameplayMgr::OnGameLeave()
{
    if (m_game)
        m_game->OnGameLeave();
}

/** \brief Nastaveni typu nove hry a herniho typu
 *
 * Zahodi stary herni typ a vytvori novy podle pozadavku
 */
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
        case GAME_TYPE_SP_MEME:
            m_game = new MemeSingleGameType;
            break;
        case GAME_TYPE_MP_CLASSIC:
            m_game = new ClassicMultiGameType;
            break;
        default:
            break;
    }

    assert(m_game != NULL);

    m_game->FillGameTypeResources();
}

/** \brief Vraci zvoleny typ hry
 */
GameType GameplayMgr::GetGameType()
{
    if (!m_game)
        return GAME_TYPE_NONE;

    return m_game->GetType();
}

/** \brief Funkce starajici se o pridani bomby
 *
 * Prida bombu na zadane souradnice ve 2D mape. Vypocita dosah, oznaci budouci nebezpecna pole a vlozi zaznam bomby do mapy
 */
bool GameplayMgr::AddBomb(uint32 x, uint32 y)
{
    if (m_plActiveBombs >= m_plMaxBombs && !IsCheatOn(CHEAT_UNLIMITED_BOMBS))
        return false;

    BombRecord* bomb = new BombRecord;
    bomb->x = x;
    bomb->y = y;
    bomb->boomTime = clock() + 2500;
    bomb->reach = IsCheatOn(CHEAT_MAX_FLAME)?10:GetFlameReach();
    bomb->sizzleSound = sSoundMgr->PlayEffect(2, true, true);
    BombMap.push_back(bomb);

    sSoundMgr->PlayEffect(25);

    m_plActiveBombs++;

    if (sGameplayMgr->GetGameType() == GAME_TYPE_SP_CLASSIC)
        localPlayerStats.ClassicSingleStats.bombsPlanted += 1;
    else if (sGameplayMgr->GetGameType() == GAME_TYPE_SP_MEME)
        localPlayerStats.MemeSingleStats.bombsPlanted += 1;

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

        sGameplayMgr->SetDangerous(bomb->x, bomb->y, bomb, tnow+2500, DEFAULT_BOMB_FLAME_DURATION);
        for (uint32 i = 1; i <= bombreach; i++)
        {
            if (reach_x1 + (rx1_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x + i, bomb->y, bomb, tnow+i*100+2500, DEFAULT_BOMB_FLAME_DURATION);
            if (reach_x2 + (rx2_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x - i, bomb->y, bomb, tnow+i*100+2500, DEFAULT_BOMB_FLAME_DURATION);
            if (reach_y1 + (ry1_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x, bomb->y + i, bomb, tnow+i*100+2500, DEFAULT_BOMB_FLAME_DURATION);
            if (reach_y2 + (ry2_box?1:0) >= i)
                sGameplayMgr->SetDangerous(bomb->x, bomb->y - i, bomb, tnow+i*100+2500, DEFAULT_BOMB_FLAME_DURATION);
        }
    }

    return true;
}

/** \brief Predcasny vybuch bomby
 *
 * Overi, zdali je pritomna bomba na zadanych souradnicich, a pokud ano, oznaci ji k predcasnemu vybuchu (zkrati cas vybuchu na minimum)
 */
void GameplayMgr::PreBoomBomb(uint32 x, uint32 y)
{
    if (BombMap.empty())
        return;

    // Projdeme vsechny bomby, a pokud na nejakou narazime, nechame ji predcasne vybouchnout
    for (std::list<BombRecord*>::iterator itr = BombMap.begin(); itr != BombMap.end(); ++itr)
    {
        if (!(*itr))
            continue;

        // Pokud se bomba nachazi na nove nebezpecnem poli, boom
        if ((*itr)->x == x && (*itr)->y == y)
        {
            // Najdeme zaznam v DangerousMap pro pole pod danou bombou
            // jeho aktivacni cas bude zdrojovy pro vsechny ostatni
            int32 timeToBomb = 0;
            for (uint32 k = 0; k < 4; k++)
            {
                if (DangerousMap[x][y][k] == NULL)
                    continue;
                if (DangerousMap[x][y][k]->origin == (*itr))
                    timeToBomb = DangerousMap[x][y][k]->activeSince - clock();
            }

            if (timeToBomb < 0)
                return;

            (*itr)->boomTime = clock();
            // Projdeme tedy vsechna jeji nebezpecna pridruzena pole
            // a zkratime jim cas aktivace na minimum
            for (int32 i = int32(x - (*itr)->reach); i < int32(x + (*itr)->reach); i++)
            {
                for (int32 j = int32(y - (*itr)->reach); j < int32(y + (*itr)->reach); j++)
                {
                    if (i < 0 || j < 0 || i >= DangerousMap.size() || j >= DangerousMap[0].size())
                        continue;

                    for (uint32 k = 0; k < 4; k++)
                    {
                        if (DangerousMap[i][j][k] == NULL)
                            continue;

                        DangerousMap[i][j][k]->activeSince -= timeToBomb;
                    }
                }
            }
        }
    }
}

/** \brief Funkce zpracovavajici udalost zmeny pole hrace
 *
 * Pouze vola metodu tridy herniho typu
 */
void GameplayMgr::OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY)
{
    if (m_game)
        m_game->OnPlayerFieldChange(oldX, oldY, newX, newY);
}

/** \brief Vraci, zdali zadane pole je nyni nebezpecne
 */
bool GameplayMgr::IsDangerousField(uint32 x, uint32 y)
{
    if (DangerousMap.size() < x)
        return false;
    if (DangerousMap[x].size() < y)
        return false;

    for (uint32 k = 0; k < 4; k++)
    {
        // Pointer v mape se rovna NULLe, kdyz neni zaznamenana zadna vybusna aktivita
        if (DangerousMap[x][y][k] == NULL)
            continue;

        if (DangerousMap[x][y][k]->activeSince <= clock() && DangerousMap[x][y][k]->activeSince+DangerousMap[x][y][k]->activeTime >= clock())
            return true;
    }

    return false;
}

/** \brief Vraci, zdali zadane pole je nebo bude nebezpecne
 */
bool GameplayMgr::WouldBeDangerousField(uint32 x, uint32 y)
{
    if (DangerousMap.size() < x)
        return false;
    if (DangerousMap[x].size() < y)
        return false;

    for (uint32 k = 0; k < 4; k++)
    {
        // Pointer v mape se rovna NULLe, kdyz neni zaznamenana zadna vybusna aktivita
        if (DangerousMap[x][y][k] != NULL)
            return true;
    }

    return false;
}

/** \brief Oznaci dane pole jako nebezpecne (nebo jestli v budoucnu bude)
 */
void GameplayMgr::SetDangerous(uint32 x, uint32 y, BombRecord* origin, clock_t since, uint32 howLong)
{
    if (x > DangerousMap.size())
        return;
    if (y > DangerousMap[x].size())
        return;

    for (uint32 k = 0; k < 4; k++)
    {
        // Musime najit volne misto na danem poli
        if (DangerousMap[x][y][k] != NULL)
            continue;

        DangerousField* field = new DangerousField;

        field->activeSince = since;
        field->activeTime = howLong;
        field->registered = false;
        field->origin = origin;

        DangerousMap[x][y][k] = field;
        break;
    }
}

/** \brief Funkce starajici se o potrebne rutiny pri splneni podminek smrti hrace
 */
void GameplayMgr::PlayerDied()
{
    if (m_playerDead)
        return;

    if (IsCheatOn(CHEAT_IMMORTAL))
        return;

    BlockMovement();

    m_playerDead = true;

    sAnimator->ChangeModelAnim(m_playerRec->AnimTicket, ANIM_DYING, 0, 0, ANIM_FLAG_NOT_REPEAT);

    sSoundMgr->PlayEffect(9);

    sApplication->SetStagePhase(3);
}

/** \brief Pauza hry
 *
 * Zastavi pohyb hrace, zapauzuje timery, emittery a pozastavi i animace, ktere je nutne zastavit
 */
void GameplayMgr::PauseGame()
{
    if (IsGamePaused())
        return;

    m_gamePaused = true;
    m_pauseTime = clock();
    sSoundMgr->MusicPause();

    BlockMovement();

    if (IsSingleGameType())
    {
        sTimer->PauseTimers();
        sParticleEmitterMgr->PauseEmitters();
        sDisplay->DisableRestrictedAnimations(ANIM_RESTRICTION_NOT_PAUSED);
    }
}

/** \brief Odpauzovani hry
 *
 * Presne opacne procedury, nez v GameplayMgr::PauseGame
 */
void GameplayMgr::UnpauseGame()
{
    if (!IsGamePaused())
        return;

    m_gamePaused = false;
    clock_t diff = clock() - m_pauseTime;
    m_pauseTime = 0;
    sSoundMgr->MusicUnpause();

    UnblockMovement();

    if (IsSingleGameType())
    {
        sTimer->UnpauseTimers();
        sParticleEmitterMgr->UnpauseEmitters();
        sDisplay->EnableRestrictedAnimations(ANIM_RESTRICTION_NOT_PAUSED);
    }

    if (!DangerousMap.empty())
    {
        for (uint32 i = 0; i < DangerousMap.size(); i++)
        {
            for (uint32 j = 0; j < DangerousMap[i].size(); j++)
            {
                for (uint32 k = 0; k < 4; k++)
                {
                    if (!DangerousMap[i][j][k])
                        continue;

                    if (DangerousMap[i][j][k]->registered)
                        DangerousMap[i][j][k]->activeTime += diff;
                    else
                        DangerousMap[i][j][k]->activeSince += diff;
                }
            }
        }
    }

    int middleX = sConfig->WindowWidth >> 1;
    int middleY = sConfig->WindowHeight >> 1;
    SetCursorPos(middleX, middleY);
}

/** \brief Vystup na herni konzoli
 *
 * Vyroluje vsechny polozky ve vystupu o jednu nahoru (jedna se tedy zahodi) a novy vypis da na novou radku
 * TODO: zalamovani --> nova privatni fce pouze s argumentem typu const char*
 */
void GameplayMgr::ConsoleWrite(const char *str, ...)
{
    va_list argList;
    va_start(argList, str);
    char buf[2048];
    vsnprintf(buf,2048,str,argList);
    va_end(argList);

    for (uint32 i = 1; i < CONSOLE_OUTPUT_LINES; i++)
        m_consoleOutput[i-1] = m_consoleOutput[i].c_str();

    m_consoleOutput[CONSOLE_OUTPUT_LINES-1] = buf;
}

/** \brief Vstup na herni konzoli (povrzeno enterem)
 *
 * Zaznamena vstup do mapy a zpracuje ho
 */
void GameplayMgr::ConsoleSubmit()
{
    if (!IsConsoleOpened())
        return;

    // = potvrzeni prazdneho radku, nema cenu handlovat dale
    if (m_consoleInput.size() == 0)
        return;

    // Odrolovat historii vstupu
    for (uint32 i = 1; i < CONSOLE_INPUT_HISTORY; i++)
        m_consoleInputHistory[i-1] = m_consoleInputHistory[i];

    // a na konec zaradit aktualni vstup
    m_consoleInputHistory[CONSOLE_INPUT_HISTORY-1] = m_consoleInput.c_str();

    // vypsat nas prikaz na konzoli
    ConsoleWrite(">> %s", m_consoleInput.c_str());

    // prikazy pouzitelne jen pri hrani
    if (sApplication->GetStage() == STAGE_GAME)
    {
        // Testovaci vystupy
        if (m_consoleInput.compare(0, m_consoleInput.size(), "POS X") == 0)
            ConsoleWrite("Position X: %f", sDisplay->GetTargetX());
        else if (m_consoleInput.compare(0, m_consoleInput.size(), "POS Y") == 0)
            ConsoleWrite("Position Y: %f", sDisplay->GetTargetY());
        else if (m_consoleInput.compare(0, m_consoleInput.size(), "POS Z") == 0)
            ConsoleWrite("Position Z: %f", sDisplay->GetTargetZ());
        // Cheaty
        else if (m_consoleInput.compare(0, m_consoleInput.size(), "JUMPOVER") == 0)
        {
            // CHEAT_NEXT_LEVEL
            // TODO: az bude hotova kampan, overit, zdali je to kampan a pripadne postoupit do dalsiho levelu
            ConsoleWrite("Next level!");
        }
        else if (m_consoleInput.compare(0, m_consoleInput.size(), "KILLKENNNY") == 0)
        {
            // CHEAT_KILL_ALL
            // TODO: projit mapu nepratel a vsechny zabit
            ConsoleWrite("All enemies killed!");
        }
        // Prepinaci cheaty
        else
        {
            for (uint32 i = 0; i < sizeof(toggleCheatMap)/sizeof(ToggleCheatDesc); i++)
            {
                if (m_consoleInput.compare(0, m_consoleInput.size(), toggleCheatMap[i].str.c_str()) == 0)
                {
                    if (m_cheatMap[toggleCheatMap[i].id] > 0)
                    {
                        ConsoleWrite("%s OFF!", toggleCheatMap[i].name.c_str());
                        m_cheatMap[toggleCheatMap[i].id] = 0;
                    }
                    else
                    {
                        ConsoleWrite("%s ON!", toggleCheatMap[i].name.c_str());
                        m_cheatMap[toggleCheatMap[i].id] = 1;
                    }
                    break;
                }
            }
        }
    }

    m_consoleInput = "";
}

/** \brief Natoceni hrace podle posunu mysi
 *
 * Jen zjisti pozici mysi, a pokud se nejak zmenila vuci stredu, pohne pohledem a mys vrati na puvodni misto
 */
void GameplayMgr::UpdatePlayerMoveAngle()
{
    if (m_movementBlocked || !sApplication->IsAppActive())
        return;

    // Otoceni hrace (hracskeho modelu) o uhel prepocitany podle pohybu mysi
    // Pri FPS rezimu musime nastavit i vertikalni uhel, ale to ve funkci Display::AdjustViewToTarget
    POINT mousePos;
    int middleX = sConfig->WindowWidth >> 1;
    int middleY = sConfig->WindowHeight >> 1;
    GetCursorPos(&mousePos);
    SetCursorPos(middleX, middleY);  // Posuneme mys zase na stred
    if (mousePos.x != middleX)
        m_moveAngle += 0.0025f*(mousePos.x-middleX);
    if (mousePos.y != middleY)
    {
        sDisplay->SetAngleX(0.02f*(mousePos.y-middleY), true);
        if (sDisplay->GetAngleX() > 40.0f)
            sDisplay->SetAngleX(40.0f);
        if (sDisplay->GetAngleX() < 20.0f)
            sDisplay->SetAngleX(20.0f);
    }

    // Update uhlu pohybu pokud se lisi od posledniho updatu o vice jak 30 stupnu
    if (fabs((180.0f * (PI/2-m_moveAngle) / PI) - m_lastUpdatedMoveAngle) > 30.0f)
        SendMoveHeartbeat();
}

/** \brief Funkce starajici se o pohyb hrace
 *
 * Pohne hracem o predem stanoveny usek, jehoz delka se lisi podle ubehnuteho casu (diff od puvodniho update)
 */
void GameplayMgr::UpdatePlayerMotion()
{
    if (!IsSingleGameType())
        UpdateOtherPlayersMotion();

    if (m_movementBlocked)
        return;

    if (IsMoving() && !IsSingleGameType() && m_lastMoveHeartbeat+1000 < clock())
        SendMoveHeartbeat();

    if (m_lastMovementUpdate < clock())
    {
        // Za jednu milisekundu musime urazit 0.002 jednotky, tzn. 1s = 2 jednotky
        float dist = (float(clock()-m_lastMovementUpdate)+1.0f)*0.002f*GetPlayerSpeedCoef();
        float angle_rad = m_moveAngle;

        m_lastMovementUpdate = clock();

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
                sAnimator->ChangeModelAnim(m_playerRec->AnimTicket, ANIM_WALK, 25, 0);

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

        if (m_forceHeartbeatSend)
        {
            SendMoveHeartbeat();
            m_forceHeartbeatSend = false;
        }
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

void GameplayMgr::UpdateOtherPlayersMotion()
{
    if (sNetwork->players.empty())
        return;

    for (PlayerList::iterator itr = sNetwork->players.begin(); itr != sNetwork->players.end(); ++itr)
    {
        if (!(*itr) || !(*itr)->rec)
            continue;

        if ((*itr)->lastMovementUpdate < clock())
        {
            float dist = (float(clock()-(*itr)->lastMovementUpdate)+1.0f)*0.002f*(*itr)->speed;
            float angle_rad = PI * (-(*itr)->rec->rotate + 90.0f) / 180.0f;

            (*itr)->lastMovementUpdate = clock();

            // nehybe se
            if (sAnimator->GetAnimId((*itr)->rec->AnimTicket) != ANIM_WALK)
                continue;

            float newx = (*itr)->rec->x + dist*cos(angle_rad);
            float newz = (*itr)->rec->z;
            uint16 collision = sDisplay->CheckCollision(newx, 0.0f, newz);

            // Pokud na tehle ose nekolidujeme, muzeme se posunout
            if (!(collision & AXIS_X))
                (*itr)->rec->x = newx;

            // Nasleduje posun po ose Z
            newx = (*itr)->rec->x;
            newz = (*itr)->rec->z + dist*sin(angle_rad);
            collision = sDisplay->CheckCollision(newx, 0.0f, newz);

            // A opet pokud nekolidujeme na dane ose, posuneme hrace
            if (!(collision & AXIS_Z))
                (*itr)->rec->z = newz;
        }
    }
}

void GameplayMgr::SetMoveElement(uint8 direction)
{
    if (direction >= MOVE_MAX)
        return;

    if (!IsSingleGameType() && !IsMoving())
    {
        SendMoveState(true);
        m_forceHeartbeatSend = true;
    } else if (direction != MOVE_FORWARD)
        m_forceHeartbeatSend = true;

    m_moveElements[direction] = true;
}

void GameplayMgr::UnsetMoveElement(uint8 direction)
{
    if (direction >= MOVE_MAX)
        return;

    m_moveElements[direction] = false;

    if (!IsSingleGameType() && !IsMoving())
    {
        SendMoveState(false);
        m_forceHeartbeatSend = true;
    } else if (direction != MOVE_FORWARD)
        m_forceHeartbeatSend = true;
}

bool GameplayMgr::IsMoving()
{
    for (uint32 i = 0; i < MOVE_MAX; i++)
        if (m_moveElements[i])
            return true;

    return false;
}

void GameplayMgr::SendMoveState(bool start)
{
    if (IsSingleGameType())
        return;

    if (start)
    {
        SmartPacket ms(CMSG_MOVE_START);
        if (GetPlayerRec())
            ms << float(GetPlayerRec()->rotate);
        else
            ms << float(0.0f);
        sNetwork->SendPacket(&ms);
    }
    else
    {
        SmartPacket ms(CMSG_MOVE_STOP);
        if (GetPlayerRec())
            ms << float(GetPlayerRec()->rotate);
        else
            ms << float(0.0f);
        sNetwork->SendPacket(&ms);
    }
}

void GameplayMgr::SendMoveHeartbeat()
{
    m_lastMoveHeartbeat = clock();

    if (!GetPlayerRec() || IsSingleGameType())
        return;

    ModelDisplayListRecord* pl = GetPlayerRec();

    SmartPacket hb(CMSG_MOVE_HEARTBEAT);

    // position
    hb << float(pl->x);
    hb << float(pl->y);
    hb << float(pl->z);

    // rotation
    hb << float(pl->rotate);

    m_lastUpdatedMoveAngle = pl->rotate;

    // speed
    hb << float(GetPlayerSpeedCoef());

    sNetwork->SendPacket(&hb);
}
