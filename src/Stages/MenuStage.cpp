#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Config.h>
#include <Network.h>

void MenuStage::OnEnter()
{
    // Vynulovat vsechny mozne uhly a podobne
    sDisplay->Initialize();
    // Menu scene
    sDisplay->DrawModel(4, 0.3f, -5.0f, -9.5f, ANIM_NONE, 1.6f);

    sDisplay->SetAngleX(20.0f);

    // Svetlo
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    GLfloat lAmbient[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat lDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lPosition[]= {0.0f, 2.0f, 0.0f, 0.8f};
    glLightfv(GL_LIGHT1, GL_AMBIENT,  lAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  lDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lSpecular);
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
    // Nastaveni
    sDisplay->PrintText(FONT_ONE, WIDTH-350, 380, FONT_SIZE_1, 0, COLOR(255,0,0), "Nastavení");

    // Credits
    sDisplay->PrintText(FONT_ONE, WIDTH-350, 450, FONT_SIZE_1, 0, COLOR(255,127,255), "Credits");

    sDisplay->PrintText(MAIN_FONT, 15,HEIGHT-15-15-15-15, FONT_SIZE_N, 0, NOCOLOR,"Verze %u.%u", 0, 1);
    sDisplay->PrintText(MAIN_FONT, 15,HEIGHT-15-15, FONT_SIZE_1, 0, NOCOLOR,"Copyright (c) Kennny 2011-2012");
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
    else if (IN_RANGE(x,y, WIDTH-350, WIDTH, 380, 380+80))
    {
        sApplication->SetStage(STAGE_OPTIONS);
        return;
    }
    else if (IN_RANGE(x,y, WIDTH-350, WIDTH, 450, 450+80))
    {
        sApplication->SetStage(STAGE_CREDITS);
        return;
    }
}
