#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Config.h>
#include <Effects/Animations.h>
#include <Effects/ParticleEmitter.h>
#include <Map.h>
#include <Gameplay.h>
#include <Timer.h>

void GameStage::OnEnter()
{
    sGameplayMgr->OnGameInit();

    // Svetla
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    GLfloat lAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat lDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lPosition[]= {0.0f, 5.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT,  lAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  lDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, lPosition);
    glEnable(GL_LIGHT1);

    // Mlha
    /*glFogi(GL_FOG_MODE, GL_LINEAR);
    GLfloat fogColor[]= {0.5f, 0.5f, 0.5f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.1f);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 3.0f);
    glFogf(GL_FOG_END, 10.0f);
    glEnable(GL_FOG);*/
}

void GameStage::OnLeave()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
}

void GameStage::OnBeforeDraw(uint32 diff)
{
    sDisplay->DrawMap();
}

void GameStage::OnDraw(uint32 diff)
{
    // Pokud existuje zaznam hrace, posuneme svetlo
    if (sGameplayMgr->GetPlayerRec())
    {
        glLoadIdentity();
        // Natoceni je dulezite, jinak se bude svetlo pohybovat stale s hracem
        glRotatef(sDisplay->GetAngleY(), 0, -1.0f, 0);
        // Souradnice se odecitaji kvuli "presunuti" nuly zpet do pocatku
        static GLfloat lPosition[] = {sGameplayMgr->GetPlayerRec()->x-5.0f, 5.0f, sGameplayMgr->GetPlayerRec()->z-5.0f, 1.5f};
        glLightfv(GL_LIGHT1, GL_POSITION, lPosition);
    }
}

void GameStage::OnKeyStateChange(uint16 key, bool press)
{
    // Jelikoz nam mys drzi pozici veprostred pohledu, nemuzeme zmacknout krizek
    // Tak si pomuzeme escapem
    if (key == VK_ESCAPE)
        exit(1);

    // Dame vedet globalnimu kontroleru, ze hrac se chce pohybovat
    if (press)
    {
        switch (key)
        {
            case 'W':
                sGameplayMgr->SetMoveElement(MOVE_FORWARD);
                break;
            case 'S':
                sGameplayMgr->SetMoveElement(MOVE_BACKWARD);
                break;
            case 'A':
                sGameplayMgr->SetMoveElement(MOVE_LEFT);
                break;
            case 'D':
                sGameplayMgr->SetMoveElement(MOVE_RIGHT);
                break;
        }
    }
    else
    {
        switch (key)
        {
            case 'W':
                sGameplayMgr->UnsetMoveElement(MOVE_FORWARD);
                break;
            case 'S':
                sGameplayMgr->UnsetMoveElement(MOVE_BACKWARD);
                break;
            case 'A':
                sGameplayMgr->UnsetMoveElement(MOVE_LEFT);
                break;
            case 'D':
                sGameplayMgr->UnsetMoveElement(MOVE_RIGHT);
                break;
        }
    }
}

void GameStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
    if (!sGameplayMgr->GetPlayerRec())
        return;

    if (left)
    {
        Map* pMap = (Map*)sMapManager->GetMap();
        if (!pMap)
            return;

        uint32 bx = floor(sGameplayMgr->GetPlayerRec()->x)+1;
        uint32 by = floor(sGameplayMgr->GetPlayerRec()->z)+1;

        if (sGameplayMgr->AddBomb(bx, by))
        {
            pMap->AddDynamicCell(bx, by, DYNAMIC_TYPE_BOMB, 0, 0, NULL);
            sMapManager->FillDynamicRecords();
            sDisplay->m_ignoreTargetCollision = DYNAMIC_TYPE_BOMB;
        }
    }
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
        sGameplayMgr->ChangePlayerMoveAngle(mousePos.x-middleX);

    // Odsud budeme updatovat i GameplayMgr, proto nesmi byt nikde predtim return
    // Nedrive pohyb
    sGameplayMgr->UpdatePlayerMotion(diff);
    // A pote zbytek
    sGameplayMgr->Update();
}
