#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Config.h>
#include <Network.h>

void MenuStage::OnEnter()
{
    // Menu scene
    sDisplay->DrawModel(4, -0.5f, -5.0f, -8.0f, ANIM_NONE, 2.0f);
    sDisplay->DrawModel(2, -4.0f, -5.0f, -13.0f, ANIM_IDLE, 1.0f);
    sDisplay->DrawModel(2, -4.0f, -5.0f, -10.0f, ANIM_IDLE, 1.0f);
    sDisplay->DrawModel(2, -4.0f, -5.0f,  -7.5f, ANIM_IDLE, 1.0f);
    sDisplay->SetAngleX(20.0f);

    // Svetlo
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    GLfloat lAmbient[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat lDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lPosition[]= {0.0f, 2.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT,  lAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  lDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, lPosition);
    glEnable(GL_LIGHT1);
}

void MenuStage::OnLeave()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
}

void MenuStage::OnDraw(uint32 diff)
{
    // Vykreslit 2D prvky
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    // Logo skoly
    sDisplay->Draw2D(32,WIDTH-144-20, HEIGHT-42-20, 144, 42);

    // Nova hra
    sDisplay->Draw2D(4 ,WIDTH-350,50 ,350,80);
    // Multiplayer
    sDisplay->Draw2D(19,WIDTH-350,160,350,80);
    // Odejit
    sDisplay->Draw2D(11,WIDTH-350,270,350,80);

    sDisplay->PrintText(MAIN_FONT, 15,HEIGHT-15-15-15-15,"Verze %u.%u", 0, 1);
    sDisplay->PrintText(SMALL_FONT, 15,HEIGHT-15-15,"Copyright (c) Kennny 2011");
}

void MenuStage::OnKeyStateChange(uint16 key, bool press)
{
    if (key == VK_UP && press)
        sDisplay->SetAngleX(-1.0f, true);
}

void MenuStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
    if (IN_RANGE(x,y, WIDTH-350, WIDTH, 50, 50+80))
    {
        sApplication->SetStage(STAGE_GAMESETTINGS, 100);
        return;
    }
    else if (IN_RANGE(x,y, WIDTH-350, WIDTH, 160, 160+80))
    {
        sNetwork->Connect(sConfig->HostName.c_str(), sConfig->NetworkPort);
        sApplication->SetStage(STAGE_GAMESETTINGS);
        return;
    }
    else if (IN_RANGE(x,y, WIDTH-350, WIDTH, 270, 270+80))
    {
        exit(0);
        return;
    }
}
