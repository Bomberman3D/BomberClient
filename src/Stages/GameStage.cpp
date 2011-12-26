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
    pPlayerRec = sDisplay->DrawModel(1, 0.5f, 0, 0.5f, ANIM_IDLE, 0.45f, 90.0f, true);
    sDisplay->SetTargetModel(pPlayerRec);

    // TODO: vytvorit "nastaveni" hry a tam tohle presunout !!
    sGameplayMgr->SetGameType(GAME_TYPE_SP_CLASSIC);


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
    if (pPlayerRec)
    {
        glLoadIdentity();
        // Natoceni je dulezite, jinak se bude svetlo pohybovat stale s hracem
        glRotatef(pPlayerRec->rotate, 0, -1.0f, 0);
        // Souradnice se odecitaji kvuli "presunuti" nuly zpet do pocatku
        static GLfloat lPosition[] = {pPlayerRec->x-5.0f, 5.0f, pPlayerRec->z-5.0f, 1.5f};
        glLightfv(GL_LIGHT1, GL_POSITION, lPosition);
    }
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
    if (left)
    {
        Map* pMap = (Map*)sMapManager->GetMap();
        if (!pMap)
            return;

        uint32 bx = floor(pPlayerRec->x)+1;
        uint32 by = floor(pPlayerRec->z)+1;

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
        pPlayerRec->rotate -= 0.05f*(mousePos.x-middleX);

    // Za jednu milisekundu musime urazit 0.002 jednotky, tzn. 1s = 2 jednotky
    float dist = (float(diff)+1.0f)*0.002f*sGameplayMgr->GetPlayerSpeedCoef();
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

    // Pripadne obslouzeni vstupu na jine pole - univerzalni check pro treba vstup do plamenu, na bonus, ...
    uint32 nX = ceil(pPlayerRec->x);
    uint32 nY = ceil(pPlayerRec->z);
    if (nX != plX || nY != plY)
    {
        sGameplayMgr->OnPlayerFieldChange(plX, plY, nX, nY);
        plX = nX;
        plY = nY;
    }

    // A nakonec vsechno prelozime tak, aby se pohled zarovnal k hraci
    // Nutne pro spravne zobrazeni
    sDisplay->AdjustViewToTarget();

    // Odsud budeme updatovat i GameplayMgr, proto nesmi byt nikde predtim return
    sGameplayMgr->Update();
}
