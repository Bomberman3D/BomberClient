#include <Global.h>
#include <GameTypes.h>
#include <Gameplay.h>
#include <Map.h>
#include <Effects/ParticleEmitter.h>

ModelDisplayListRecord* MemeSingleGameType::SpawnNewPlayer()
{
    ModelDisplayListRecord* tmp = sDisplay->DrawModel(11, 0.5f, 0, 0.5f, ANIM_IDLE, 3.5f, 90.0f, true, false, 0, 0, ANIM_RESTRICTION_NOT_PAUSED);
    sDisplay->AddModelFeature(tmp, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(65, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));

    return tmp;
}

void MemeSingleGameType::FillGameTypeResources()
{
    gameResources.PlayerModelIDs.push_back(11);

    gameResources.EnemyModelIDs.push_back(11);

    // 62-69 jsou textury rage comics obliceju
    for (uint32 i = 62; i <= 69; i++)
        gameResources.MiscTextureIDs.push_back(i);
}

void MemeSingleGameType::OnGameInit(ModelDisplayListRecord* pPlayerRec)
{
    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    m_enemies.clear();

    pMap->DestroyAllDynamicRecords();

    uint32 startlocpos = 0; // vzdy je jedna vyhrazena pro hrace + se inkrementuje s poctem nepratel

    for (uint32 i = 0; i < pMap->field.size(); i++)
    {
        for (uint32 j = 0; j < pMap->field[0].size(); j++)
        {
            // Muzeme dat bednu jen na misto kde nebude stat hrac, ani pevny objekt mapy
            if (pMap->field[i][j].type == TYPE_GROUND && !pMap->NearStartPos(i,j))
            {
                // pri 10ti se deli dvema, pri 1 se deli 11ti, asi nejjednodussi
                if ((rand()%(12-sGameplayMgr->GetSetting(SETTING_BOX_DENSITY))) == 0)
                {
                    pMap->AddDynamicCell(i,j,DYNAMIC_TYPE_BOX);
                }
            }
        }
    }

    sMapManager->FillDynamicRecords();

    uint32 enemycount = 0;

    for (uint32 i = 0; i < pMap->field.size(); i++)
    {
        for (uint32 j = 0; j < pMap->field[0].size(); j++)
        {
            if (pMap->field[i][j].type == TYPE_STARTLOC)
            {
                // Pokud je vetsi nez 0, muzeme tam dat nepritele, jinak se jedna o hracovu pozici
                if (startlocpos > 0 && startlocpos <= sGameplayMgr->GetSetting(SETTING_ENEMY_COUNT))
                {
                    // TODO: lepsi vyber modelu a tak.. asi to bude lepsi presunout do funkce
                    // TODO2: vyber AI urovne podle zvolene v nastaveni.. asi derivovat podtridu, ale je to fuk, de to zapodminkovat
                    MemeEnemy* pEnemy = new MemeEnemy;
                    pEnemy->Init(gameResources.EnemyModelIDs[(rand()% gameResources.EnemyModelIDs.size() )], i, j, ++enemycount);
                    pEnemy->m_movement->SetSpeedMod(1.0f - (float(sGameplayMgr->GetSetting(SETTING_ENEMY_SPEED)) / 10.0f));
                    pEnemy->m_movement->Mutate(MOVEMENT_TARGETTED);
                    m_enemies.push_back(pEnemy);
                    startlocpos++;
                }
                else
                {
                    pPlayerRec->x = i-0.5f;
                    pPlayerRec->z = j-0.5f;
                    startlocpos++;
                }
            }
        }
    }

    sGameplayMgr->SetGameEndTime(clock() + 2 * 60 * 1000); // 2 minuty
}

void MemeSingleGameType::OnGameLeave()
{
    if (!m_enemies.empty())
    {
        for (std::list<EnemyTemplate*>::iterator itr = m_enemies.begin(); itr != m_enemies.end(); )
        {
            if ((*itr)->pRecord)
            {
                (*itr)->pRecord->remove = true;
                (*itr)->pRecord = NULL;
            }
            itr = m_enemies.erase(itr);
        }
    }
}

void MemeSingleGameType::OnUpdate()
{
    if (!m_enemies.empty())
    {
        for (std::list<EnemyTemplate*>::iterator itr = m_enemies.begin(); itr != m_enemies.end(); )
        {
            // Pokud neexistuje cil, vymazeme zaznam z vektoru
            if (!(*itr))
            {
                itr = m_enemies.erase(itr);
                continue;
            }

            (*itr)->Update();

            if ((*itr)->pRecord && sDisplay->ModelIntersection(sGameplayMgr->GetPlayerRec(), (*itr)->pRecord))
                sGameplayMgr->PlayerDied();

            ++itr;
        }
    }
}

void MemeSingleGameType::OnBombBoom(BombRecord* bomb)
{
    if (!bomb)
        return;

    Map* pMap = (Map*)sMapManager->GetMap();
    if (pMap)
    {
        // Nejdrive znicime bombu na mape
        pMap->DestroyDynamicRecords(bomb->x, bomb->y, DYNAMIC_TYPE_BOMB);

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

        // Exploze - particle emittery
        BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0,   0.0f, 0, 0, reach_y1*100 + (ry1_box?100:0), 20, 10.0f, 0.1f, 50, 10, 1, 0, ANIM_FLAG_NOT_REPEAT, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0,  90.0f, 0, 0, reach_x2*100 + (rx2_box?100:0), 20, 10.0f, 0.1f, 50, 10, 1, 0, ANIM_FLAG_NOT_REPEAT, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0, 180.0f, 0, 0, reach_y2*100 + (ry2_box?100:0), 20, 10.0f, 0.1f, 50, 10, 1, 0, ANIM_FLAG_NOT_REPEAT, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0, 270.0f, 0, 0, reach_x1*100 + (rx1_box?100:0), 20, 10.0f, 0.1f, 50, 10, 1, 0, ANIM_FLAG_NOT_REPEAT, 1500);

        // Casovani vybuchu je implementovano pri polozeni bomby
    }
}

void MemeSingleGameType::OnBoxDestroy(uint32 x, uint32 y, bool by_bomb)
{
    BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
    sParticleEmitterMgr->AddEmitter(templ, x-0.5f, 0.0f, y-0.5f, 0.6f, 0.6f, 90.0f, 0.0f, 0, 0, 150, 30, 5.0f, 0.1f, 10, 5, 0, 0, 0, 1500);

    if (by_bomb)
    {
        // vytvorit bonus na dynamic mape
        Map* pMap = (Map*)sMapManager->GetMap();
        if (!pMap)
            return;

        // sance 1:1, pozdeji asi doladit
        if (rand()%2 > 0)
        {
            uint32 which = rand()%3;
            pMap->AddDynamicCell(x,y,DYNAMIC_TYPE_BONUS,0,which);
            sMapManager->FillDynamicRecords();
        }
    }

    sGameplayMgr->localPlayerStats.MemeSingleStats.boxesDestroyed += 1;
}

void MemeSingleGameType::OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY)
{
    m_playerX = newX;
    m_playerY = newY;

    if (sGameplayMgr->IsDangerousField(newX, newY))
    {
        // Obsluha smrti - predame to zpet
        sGameplayMgr->PlayerDied();
    }

    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    Map::DynamicCellSet* pSet = pMap->GetDynamicCellSet(newX, newY);
    if (!pSet || pSet->empty())
        return;

    for (Map::DynamicCellSet::iterator itr = pSet->begin(); itr != pSet->end(); ++itr)
    {
        if (itr->type == DYNAMIC_TYPE_BONUS)
        {
            switch (itr->misc)
            {
                case BONUS_FLAME:
                    if (sGameplayMgr->GetFlameReach() < 5)
                        sGameplayMgr->SetFlameReach(1, true);
                    break;
                case BONUS_SPEED:
                    if (sGameplayMgr->GetPlayerSpeedCoef() < 2.0f)
                        sGameplayMgr->SetPlayerSpeedCoef(0.2f, true);
                    break;
                case BONUS_BOMB:
                    if (sGameplayMgr->GetMaxBombs() < 6)
                        sGameplayMgr->SetMaxBombs(1, true);
                    break;
            }
        }
    }

    pMap->DestroyDynamicRecords(newX, newY, DYNAMIC_TYPE_BONUS);
}

void MemeSingleGameType::OnDangerousFieldActivate(uint32 x, uint32 y)
{
    Map* pMap = (Map*)sMapManager->GetMap();
    if (pMap)
    {
        pMap->DestroyDynamicRecords(x, y, DYNAMIC_TYPE_BONUS);
        pMap->DestroyDynamicRecords(x, y, DYNAMIC_TYPE_BOX);
    }

    // Pokud hrac stoji na nebezpecnem poli
    if (m_playerX == x && m_playerY == y)
    {
        // GameplayMgr se postara o zbytek.. ouch!
        sGameplayMgr->PlayerDied();
    }

    // Projit vsechny nepratele, zdali nestoji na miste s ohnem
    for (std::list<EnemyTemplate*>::iterator itr = m_enemies.begin(); itr != m_enemies.end();)
    {
        // Znicit nepritele pokud je na poli s vybuchem
        if (!(*itr)->IsDead() && ceil((*itr)->pRecord->x) == x && ceil((*itr)->pRecord->z) == y)
        {
            (*itr)->SetDead(true);
            (*itr)->m_movement->Mutate(MOVEMENT_NONE);

            sGameplayMgr->localPlayerStats.MemeSingleStats.enemiesTrolled += 1;

            // TODO: animace smrti

            itr = m_enemies.erase(itr);
            continue;
        }
        ++itr;
    }
}
