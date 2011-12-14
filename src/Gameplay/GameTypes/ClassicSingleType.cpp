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
        uint32 bombreach = 2;

        // Projdeme vsechny mozne zaznamy na mape az na maximalni definovany dosah
        // pokud tam neco je, zamezime dalsimu prepsani a zapiseme
        uint32 reach_x1 = bombreach, reach_x2 = bombreach, reach_y1 = bombreach, reach_y2 = bombreach;
        for (uint32 i = 0; i < bombreach; i++)
        {
            // Pokud jeste nebylo zapsano
            if (reach_x1 == bombreach)
                // Overime, zdali na dane pozici neco neprostupneho je
                if (pMap->IsDynamicRecordPresent(bomb->x + i, bomb->y, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x+i,bomb->y) == TYPE_SOLID_BOX)
                    // Pokud ano, zamezime zapsanim maximalni pozice dalsimu prepsani
                    reach_x1 = i;

            if (reach_x2 == bombreach)
                if (pMap->IsDynamicRecordPresent(bomb->x - i, bomb->y, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x - i,bomb->y) == TYPE_SOLID_BOX)
                    reach_x2 = i;

            if (reach_y1 == bombreach)
                if (pMap->IsDynamicRecordPresent(bomb->x, bomb->y + i, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x,bomb->y+i) == TYPE_SOLID_BOX)
                    reach_y1 = i;

            if (reach_y2 == bombreach)
                if (pMap->IsDynamicRecordPresent(bomb->x, bomb->y - i, DYNAMIC_TYPE_BOX) || pMap->GetStaticRecord(bomb->x,bomb->y-i) == TYPE_SOLID_BOX)
                    reach_y2 = i;
        }
        // Pak vsechno dekrementujeme - zapise se vzdy pozice uz nepristupneho pole
        --reach_x1;
        --reach_x2;
        --reach_y1;
        --reach_y2;

        // Exploze - particle emittery
        BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 1.0f, 1.0f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.1f, 0.3f, 0, 0, 0, 0, reach_y1*100, 20, 10.0f, 0.1f, 100, 10, 0, 0, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 1.0f, 1.0f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.1f, 0.3f, 90.0f, 0, 0, 0, reach_x2*100, 20, 10.0f, 0.1f, 100, 10, 0, 0, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 1.0f, 1.0f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.1f, 0.3f, 180.0f, 0, 0, 0, reach_y2*100, 20, 10.0f, 0.1f, 100, 10, 0, 0, 1500);

        templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 1.0f, 1.0f, true, true);
        sParticleEmitterMgr->AddEmitter(templ, bomb->x-0.5f, 0.1f, bomb->y-0.5f, 0.1f, 0.3f, 270.0f, 0, 0, 0, reach_x1*100, 20, 10.0f, 0.1f, 100, 10, 0, 0, 1500);
    }
}
