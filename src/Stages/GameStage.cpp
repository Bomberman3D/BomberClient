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
    sDisplay->SetTargetX(-0.5f);
    sDisplay->SetTargetZ(-0.5f);
    sDisplay->SetTargetY(0);

    pPlayerRec = sDisplay->DrawModel(1, 0.5f, 0, 0.5f, ANIM_IDLE, 0.18f, 90.0f);
    sDisplay->SetTargetModel(pPlayerRec);

    // TEST !!!
    Map* pMap = (Map*)sMapManager->GetMap();
    pMap->AddDynamicCell(3,3,DYNAMIC_TYPE_BOX, 0, 0, NULL);
    pMap->AddDynamicCell(3,2,DYNAMIC_TYPE_BOMB, 0, 0, NULL);

    DisplayListRecord* pbill = sDisplay->DrawBillboard(21,0.5f,0,0.5f, 1, 1, 0.5f, 0.5f, true, true);

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
    if (key == VK_ESCAPE)
        exit(1);

    if (!pPlayerRec)
        return;

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
    POINT mousePos;
    int middleX = sConfig->WindowWidth >> 1;
    int middleY = sConfig->WindowHeight >> 1;
    GetCursorPos(&mousePos);
    SetCursorPos(middleX, middleY);
    if (!( (mousePos.x == middleX) && (mousePos.y == middleY) ))
    {
        pPlayerRec->rotate -= 0.05f*(mousePos.x-middleX);
        //Pouze v FPS !
        //SetVAngle( 0.05f*( mousePos.y-middleY),true);
    }

    // Za jednu milisekundu musíme urazit 0.0125 jednotky
    // TODO: "zkrasnit"
    float dist = ((float(diff)/5.0f+1.0f)/10.0f)*0.125f;
    float angle_rad = PI*(-pPlayerRec->rotate+90.0f)/180.0f;

    if (sApplication->IsKeyPressed('W'))
    {
        float newx = pPlayerRec->x + dist*cos(angle_rad);
        float newz = pPlayerRec->z;
        uint16 collision = sDisplay->CheckCollision(newx, 0.0f, newz);

        if (!(collision & AXIS_X))
            pPlayerRec->x = newx;

        newx = pPlayerRec->x;
        newz = pPlayerRec->z + dist*sin(angle_rad);
        collision = sDisplay->CheckCollision(newx, 0.0f, newz);

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

    sDisplay->AdjustViewToTarget();
}
