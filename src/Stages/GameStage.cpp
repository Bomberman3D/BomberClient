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
    m_subStage = 0;

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
    sGameplayMgr->OnGameLeave();

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
        glRotatef(sDisplay->GetAngleY(), 0, 1.0f, 0);
        // Souradnice se odecitaji kvuli "presunuti" nuly zpet do pocatku
        static GLfloat lPosition[] = {sGameplayMgr->GetPlayerRec()->x-5.0f, 5.0f, sGameplayMgr->GetPlayerRec()->z-5.0f, 1.5f};
        glLightfv(GL_LIGHT1, GL_POSITION, lPosition);
    }

    POINT mousePos;
    mousePos.x = sApplication->GetMouseX();
    mousePos.y = sApplication->GetMouseY();

    // Hra zapauzovana
    if (m_subStage == 2)
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        sDisplay->Draw2D(15, 150, 80, WIDTH-150-150, 320);
        sDisplay->PrintText(FONT_ONE, WIDTH/2-38*4.5f*FONT_SIZE_1, 130, FONT_SIZE_1, NOCOLOR, "P a u z a");

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*5.5f*FONT_SIZE_3, WIDTH/2+38*5.5f*FONT_SIZE_3, 250-5, 250+35))
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250    , FONT_SIZE_3, COLOR(200, 200, 0), "Zpìt do hry");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250    , FONT_SIZE_3, COLOR(200, 0, 0), "Zpìt do hry");

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*5.5f*FONT_SIZE_3,  WIDTH/2+38*5.5f*FONT_SIZE_3, 250+35, 250+95))
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250+60 , FONT_SIZE_3, COLOR(200, 200, 0), "Ukonèit hru");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250+60 , FONT_SIZE_3, COLOR(200, 0, 0), "Ukonèit hru");

        sDisplay->Setup3DMode();
    }
    // Game over!
    else if (m_subStage == 3)
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        // 38 px na 1 znak
        sDisplay->Draw2D(14, WIDTH/8, 2*HEIGHT/7, 3*WIDTH/4, 100);
        sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5, 2*HEIGHT/7+25, FONT_SIZE_N, COLOR(255, 0, 0), "Jsi mrtev!");

        sDisplay->Draw2D(15, WIDTH/4, 2*HEIGHT/7+100, WIDTH/2, 160);

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*6*0.6f, WIDTH/2+38*6*0.6f, 2*HEIGHT/7+125, 2*HEIGHT/7+125+60))
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5*0.6f, 2*HEIGHT/7+125, FONT_SIZE_2, COLOR(255, 255, 0)  , "Pokraèovat");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5*0.6f, 2*HEIGHT/7+125, FONT_SIZE_2, COLOR(255, 0, 255), "Pokraèovat");

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*6*0.6f, WIDTH/2+38*6*0.6f, 2*HEIGHT/7+125+60, 2*HEIGHT/7+125+130))
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*0.6f, 2*HEIGHT/7+125+70, FONT_SIZE_2, COLOR(255, 255, 0)  , "Hlavní menu");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*0.6f, 2*HEIGHT/7+125+70, FONT_SIZE_2, COLOR(255, 0, 255), "Hlavní menu");

        sDisplay->Setup3DMode();
    }
    // Statistiky (po smrti)
    else if (m_subStage == 4)
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        sDisplay->Draw2D(15, 0, 0, WIDTH, HEIGHT);

        // Singleplayer only
        // TODO: multiplayer stats (vice hracu, ...)

        sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5, 50, FONT_SIZE_N, COLOR(255,0,0), "Statistiky");

        sDisplay->PrintText(FONT_ONE, 1*HEIGHT/8, 150, FONT_SIZE_4, COLOR(255,255,255), PlayerStatsNames[sGameplayMgr->GetGameType()][0]);
        sDisplay->PrintText(MAIN_FONT, 1*HEIGHT/8+20, 150+50, FONT_SIZE_H3, COLOR(255,255,255), "%u", sGameplayMgr->localPlayerStats.UniversalStats.field1);
        sDisplay->PrintText(FONT_ONE, 3*HEIGHT/8, 150, FONT_SIZE_4, COLOR(255,255,255), PlayerStatsNames[sGameplayMgr->GetGameType()][1]);
        sDisplay->PrintText(MAIN_FONT, 3*HEIGHT/8+20, 150+50, FONT_SIZE_H3, COLOR(255,255,255), "%u", sGameplayMgr->localPlayerStats.UniversalStats.field2);
        sDisplay->PrintText(FONT_ONE, 5*HEIGHT/8, 150, FONT_SIZE_4, COLOR(255,255,255), PlayerStatsNames[sGameplayMgr->GetGameType()][2]);
        sDisplay->PrintText(MAIN_FONT, 5*HEIGHT/8+20, 150+50, FONT_SIZE_H3, COLOR(255,255,255), "%u", sGameplayMgr->localPlayerStats.UniversalStats.field3);

        if (IN_RANGE(mousePos.x, mousePos.y, 50, 50+38*12, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            sDisplay->PrintText(FONT_ONE, 50,                         HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, COLOR(255, 255, 0), "Zkusit znovu");
        else
            sDisplay->PrintText(FONT_ONE, 50,                         HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, COLOR(255, 0, 0), "Zkusit znovu");

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH-50-38*12*FONT_SIZE_2, WIDTH-50, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            sDisplay->PrintText(FONT_ONE, WIDTH-50-38*12*FONT_SIZE_2, HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, COLOR(255, 255, 0), "Zpìt do menu");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH-50-38*12*FONT_SIZE_2, HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, COLOR(255, 0, 0), "Zpìt do menu");

        sDisplay->Setup3DMode();
    }
}

void GameStage::OnKeyStateChange(uint16 key, bool press)
{
    // Menu (pauza)
    if (key == VK_ESCAPE && press)
    {
        // Dovolime prepnuti pouze kdyz jsme zapauzovani nebo hrajeme
        if (m_subStage == 2)
        {
            // Nejdriv nastavit pozici mysi doprostred, abychom zamezili ostremu otoceni
            int middleX = sConfig->WindowWidth >> 1;
            int middleY = sConfig->WindowHeight >> 1;
            SetCursorPos(middleX, middleY);

            sGameplayMgr->UnblockMovement();
            m_subStage = 0;
        }
        else if (m_subStage == 0)
        {
            sGameplayMgr->BlockMovement();
            m_subStage = 2;
        }
    }

    // Pokud je substage rovna 2 (menu), zadny pohyb !
    if (m_subStage == 2)
        return;

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
        // bloky if .. else if .. pouzity kvuli prehlednosti

        // Pokladame bomby pouze kdyz hrajeme (ne v menu nebo kdyz jsme mrtvi)
        if(m_subStage == 0)
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
        // jsme zapauzovani
        else if (m_subStage == 2)
        {
            // Zpet do hry
            if (IN_RANGE(x, y, WIDTH/2-38*5.5f*FONT_SIZE_3, WIDTH/2+38*5.5f*FONT_SIZE_3, 250-5, 250+35))
            {
                // Nejdriv nastavit pozici mysi doprostred, abychom zamezili ostremu otoceni
                int middleX = sConfig->WindowWidth >> 1;
                int middleY = sConfig->WindowHeight >> 1;
                SetCursorPos(middleX, middleY);

                sGameplayMgr->UnblockMovement();
                m_subStage = 0;
            }
            // Ukoncit hru
            else if (IN_RANGE(x, y, WIDTH/2-38*5.5f*FONT_SIZE_3,  WIDTH/2+38*5.5f*FONT_SIZE_3, 250+35, 250+95))
            {
                m_subStage = 4;
            }
        }
        // jsme mrtvi
        else if (m_subStage == 3)
        {
            // Pokracovat
            if (IN_RANGE(x, y, WIDTH/2-38*6*0.6f, WIDTH/2+38*5.5f*0.6f, 2*HEIGHT/7+125, 2*HEIGHT/7+125+60))
            {
                // Faze s vysledky a menu
                m_subStage = 4;
            }
            // Zpìt do menu
            else if (IN_RANGE(x, y, WIDTH/2-38*6*0.6f, WIDTH/2+38*5.5f*0.6f, 2*HEIGHT/7+125+60, 2*HEIGHT/7+125+130))
            {
                sApplication->SetStage(STAGE_MENU);
            }
        }
        // jsme ve statistikach
        else if (m_subStage == 4)
        {
            if (IN_RANGE(x, y, 50, 50+38*12, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            {
                sApplication->SetStage(STAGE_LOADING, 1);
            }
            else if (IN_RANGE(x, y, WIDTH-50-38*12*FONT_SIZE_2, WIDTH-50, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            {
                sApplication->SetStage(STAGE_MENU);
            }
        }
    }
}

void GameStage::OnUpdate(uint32 diff)
{
    // Odsud budeme updatovat i GameplayMgr
    // Nejdrive uhel podle mysi
    sGameplayMgr->UpdatePlayerMoveAngle();
    // Pak pohyb
    sGameplayMgr->UpdatePlayerMotion(diff);
    // A pote zbytek
    sGameplayMgr->Update();
}
