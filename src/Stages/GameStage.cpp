#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Config.h>
#include <Effects/Animations.h>
#include <Effects/ParticleEmitter.h>
#include <Map.h>

void GameStage::OnEnter()
{
    pPlayerRec = sDisplay->DrawModel(1, 0.5f, 0, 0.5f, ANIM_IDLE, 0.18f, 90.0f, true);
    sDisplay->SetTargetModel(pPlayerRec);

    // TEST !!!
    Map* pMap = (Map*)sMapManager->GetMap();
    pMap->AddDynamicCell(3,3,DYNAMIC_TYPE_BOX, 0, 0, NULL);
    pMap->AddDynamicCell(3,2,DYNAMIC_TYPE_BOMB, 0, 0, NULL);

    DisplayListRecord* pbill = BillboardDisplayListRecord::Create(21,0.5f,0,0.5f, 0.5f, 0.5f, true, true);

    sParticleEmitterMgr->AddEmitter(pbill, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 45.0f, 30.0f, 0.0f, 0.0f, 10000, 1000, 4.0f, 1.0f, 50, 10, 1, 1, 10000);
    // END OF TEST !!!

    sMapManager->FillDynamicRecords();
}

void GameStage::OnBeforeDraw(uint32 diff)
{
    sDisplay->DrawMap();
}

void GameStage::OnDraw(uint32 diff)
{
}

void GameStage::OnKeyStateChange(uint16 key, bool press)
{
    // Jelikoz nam mys drzi pozici veprostred pohledu, nemuzeme zmacknout krizek
    // Tak si pomuzeme escapem
    if (key == VK_ESCAPE)
        exit(1);

    if (!pPlayerRec)
        return;

    // Zmena animace chuze hrace
    if (key == 'W')
    {
        if (press)
            sAnimator->ChangeModelAnim(pPlayerRec->AnimTicket, ANIM_WALK, 0, 5);
        else
            sAnimator->ChangeModelAnim(pPlayerRec->AnimTicket, ANIM_IDLE, 0, 0);
    }
}

void GameStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
}

void GameStage::OnUpdate(uint32 diff)
{
    // Otoceni hrace (hracskeho modelu) o uhel prepocitany podle pohybu mysi
    // Pri FPS rezimu musime nastavit i vertikalni uhel, ale to ve funkci Display::AdjustViewToTarget
    POINT mousePos;
    int middleX = sConfig->WindowWidth >> 1;
    int middleY = sConfig->WindowHeight >> 1;
    GetCursorPos(&mousePos);
    SetCursorPos(middleX, middleY);  // Posuneme mys zase na stred
    if (!((mousePos.x == middleX) && (mousePos.y == middleY)))
        pPlayerRec->rotate -= 0.05f*(mousePos.x-middleX);

    // Za jednu milisekundu musime urazit 0.002 jednotky, tzn. 1s = 2 jednotky
    float dist = (float(diff)+1.0f)*0.002f;
    float angle_rad = PI*(-pPlayerRec->rotate+90.0f)/180.0f;

    // Pohyby postavy pomoci klavesnice
    // Bude zdokumentovan jen jeden blok, ostatni jsou stejne
    if (sApplication->IsKeyPressed('W'))
    {
        // Nejdrive se zkontroluje kolize na ose X
        float newx = pPlayerRec->x + dist*cos(angle_rad);
        float newz = pPlayerRec->z;
        uint16 collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        // Pokud na tehle ose nekolidujeme, muzeme se posunout
        if (!(collision & AXIS_X))
            pPlayerRec->x = newx;

        // Nasleduje posun po ose Z
        newx = pPlayerRec->x;
        newz = pPlayerRec->z + dist*sin(angle_rad);
        collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        // A opet pokud nekolidujeme na dane ose, posuneme hrace
        if (!(collision & AXIS_Z))
            pPlayerRec->z = newz;
    }
    if (sApplication->IsKeyPressed('S'))
    {
        float newx = pPlayerRec->x - dist*cos(angle_rad);
        float newz = pPlayerRec->z;
        uint16 collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        if (!(collision & AXIS_X))
            pPlayerRec->x = newx;

        newx = pPlayerRec->x;
        newz = pPlayerRec->z - dist*sin(angle_rad);
        collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        if (!(collision & AXIS_Z))
            pPlayerRec->z = newz;
    }
    if (sApplication->IsKeyPressed('A'))
    {
        float newx = pPlayerRec->x + dist*cos(angle_rad-PI/2);
        float newz = pPlayerRec->z;
        uint16 collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        if (!(collision & AXIS_X))
            pPlayerRec->x = newx;

        newx = pPlayerRec->x;
        newz = pPlayerRec->z + dist*sin(angle_rad-PI/2);
        collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        if (!(collision & AXIS_Z))
            pPlayerRec->z = newz;
    }
    if (sApplication->IsKeyPressed('D'))
    {
        float newx = pPlayerRec->x + dist*cos(angle_rad+PI/2);
        float newz = pPlayerRec->z;
        uint16 collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        if (!(collision & AXIS_X))
            pPlayerRec->x = newx;

        newx = pPlayerRec->x;
        newz = pPlayerRec->z + dist*sin(angle_rad+PI/2);
        collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        if (!(collision & AXIS_Z))
            pPlayerRec->z = newz;
    }

    // A nakonec vsechno prelozime tak, aby se pohled zarovnal k hraci
    // Nutne pro spravne zobrazeni
    sDisplay->AdjustViewToTarget();
}
