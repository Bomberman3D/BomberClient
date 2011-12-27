#include <Global.h>
#include <GameTypes.h>
#include <Gameplay.h>
#include <Map.h>
#include <Effects/ParticleEmitter.h>

void ClassicSingleGameType::OnGameInit()
{
    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    pMap->DestroyAllDynamicRecords();

    for (uint32 i = 0; i < pMap->field.size(); i++)
    {
        for (uint32 j = 0; j < pMap->field[i].size(); j++)
        {
            // Muzeme dat bednu jen na misto kde nebude stat hrac, ani pevny objekt mapy
            if ((i > 2 || j > 2) && pMap->field[i][j].type == TYPE_GROUND)
            {
                if (rand()%2)
                {
                    pMap->AddDynamicCell(i,j,DYNAMIC_TYPE_BOX);
                }
            }
        }
    }

    sMapManager->FillDynamicRecords();
}

void ClassicSingleGameType::OnUpdate()
{
    if (!m_enemies.empty())
    {
        clock_t tnow = clock();

        for (std::list<EnemyTemplate*>::iterator itr = m_enemies.begin(); itr != m_enemies.end(); )
        {
            // Pokud neexistuje cil, vymazeme zaznam z vektoru
            if (!(*itr))
            {
                itr = m_enemies.erase(itr);
                continue;
            }
            // Pokud jeste neni cas na update, posuneme se dal
            if ((*itr)->m_nextUpdate > tnow)
            {
                ++itr;
                continue;
            }

            // Samotny update
        }
    }
}

void ClassicSingleGameType::OnBombBoom(BombRecord* bomb)
{
    if (!bomb)
        return;

    Map* pMap = (Map*)sMapManager->GetMap();
    if (pMap)
    {
        // Nejdrive znicime bombu na mape
        pMap->DestroyDynamicRecords(bomb->x, bomb->y, DYNAMIC_TYPE_BOMB);

        // Tohle bude v budoucnu promenliva hodnota, podle bonusu apod.
        uint32 bombreach = sGameplayMgr->GetFlameReach();

        // Projdeme vsechny mozne zaznamy na mape az na maximalni definovany dosah
        // pokud tam neco je, zamezime dalsimu prepsani a zapiseme
        uint32 reach_x1 = bombreach+1, reach_x2 = bombreach+1, reach_y1 = bombreach+1, reach_y2 = bombreach+1;
        // Promenna urcujici, zdali doslo ke kolizi kvuli objektu (true) nebo kvuli dosahu (false)
        bool rx1_box = false, rx2_box = false, ry1_box = false, ry2_box = false;

        for (uint32 i = 0; i < bombreach; i++)
        {
            // Pokud jeste nebylo zapsano
            if (reach_x1 == bombreach+1)
                // Overime, zdali na dane pozici neco neprostupneho je
                if (pMap->IsDynamicRecordPresent(bomb->x + i + 1, bomb->y, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x+i+1,bomb->y) == TYPE_SOLID_BOX)
                {
                    // Pokud ano, zamezime zapsanim maximalni pozice dalsimu prepsani
                    reach_x1 = i+1;
                    rx1_box = true;
                }

            if (reach_x2 == bombreach+1)
                if (pMap->IsDynamicRecordPresent(bomb->x - i - 1, bomb->y, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x - i - 1,bomb->y) == TYPE_SOLID_BOX)
                {
                    reach_x2 = i+1;
                    rx2_box = true;
                }

            if (reach_y1 == bombreach+1)
                if (pMap->IsDynamicRecordPresent(bomb->x, bomb->y + i + 1, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x,bomb->y+i+1) == TYPE_SOLID_BOX)
                {
                    reach_y1 = i+1;
                    ry1_box = true;
                }

            if (reach_y2 == bombreach+1)
                if (pMap->IsDynamicRecordPresent(bomb->x, bomb->y - i - 1, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x,bomb->y-i-1) == TYPE_SOLID_BOX)
                {
                    reach_y2 = i+1;
                    ry2_box = true;
                }
        }
        // Pak vsechno dekrementujeme - zapise se vzdy pozice uz nepristupneho pole
        --reach_x1;
        --reach_x2;
        --reach_y1;
        --reach_y2;

        // Exploze - particle emittery
        BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0,   0.0f, 0, 0, reach_y1*100, 20, 10.0f, 0.1f, 50, 10, 0, 0, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0,  90.0f, 0, 0, reach_x2*100, 20, 10.0f, 0.1f, 50, 10, 0, 0, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0, 180.0f, 0, 0, reach_y2*100, 20, 10.0f, 0.1f, 50, 10, 0, 0, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.15f, 0.4f, 0, 270.0f, 0, 0, reach_x1*100, 20, 10.0f, 0.1f, 50, 10, 0, 0, 1500);

        // A nakonec znicime vsechny bedny v dosahu bomby (v pripade true hodnoty bool promenne +1 / -1 proto, ze
        // reach je definovan pro dosah plamene. Bedna je o jedno pole dal)
        pMap->DestroyDynamicRecords(bomb->x + reach_x1 + (rx1_box?1:0), bomb->y, DYNAMIC_TYPE_BOX);
        pMap->DestroyDynamicRecords(bomb->x - reach_x2 - (rx2_box?1:0), bomb->y, DYNAMIC_TYPE_BOX);
        pMap->DestroyDynamicRecords(bomb->x, bomb->y + reach_y1 + (ry1_box?1:0), DYNAMIC_TYPE_BOX);
        pMap->DestroyDynamicRecords(bomb->x, bomb->y - reach_y2 - (rx1_box?1:0), DYNAMIC_TYPE_BOX);
    }
}

void ClassicSingleGameType::OnBoxDestroy(uint32 x, uint32 y, bool by_bomb)
{
    BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.8f, 0.8f, true, true);
    sParticleEmitterMgr->AddEmitter(templ, x-0.5f, 0.0f, y-0.5f, 0.6f, 0.6f, 90.0f, 0.0f, 0, 0, 150, 30, 5.0f, 0.1f, 10, 5, 0, 0, 1500);

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
}

void ClassicSingleGameType::OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY)
{
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
