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

    glClearColor(0.05f, 0.2f, 0.0f, 0.0f);

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

    float mult = 0.5f;
    if (WIDTH > 1024 && HEIGHT > 768)
        mult = 1.0f;
    if (WIDTH > 800 && HEIGHT > 600)
        mult = 0.85f;
    else if (WIDTH > 640 && HEIGHT > 480)
        mult = 0.7f;

    sDisplay->Draw2D(3, 0, 0, 640*mult, 624*mult);
    sDisplay->Draw2D(1, WIDTH-(640*mult), HEIGHT-(624*mult), 640*mult, 624*mult);

    // Logo skoly
    sDisplay->Draw2D(32,WIDTH-144-20, HEIGHT-42-20, 144, 42);

    sDisplay->PrintText(FONT_ONE, WIDTH-300*WIDTH/800, 70*HEIGHT/600, FONT_SIZE_1, 0, NOCOLOR, "Nová hra");
    sDisplay->PrintText(FONT_ONE, WIDTH-340*WIDTH/800, 180*HEIGHT/600, FONT_SIZE_1, 0, NOCOLOR, "Multiplayer");
    sDisplay->PrintText(FONT_ONE, WIDTH-390*WIDTH/800, 285*HEIGHT/600, FONT_SIZE_1, 0, NOCOLOR, "Nastavení");
    sDisplay->PrintText(FONT_ONE, WIDTH-490*WIDTH/800, 380*HEIGHT/600, FONT_SIZE_1, 0, NOCOLOR, "Autoøi");
    sDisplay->PrintText(FONT_ONE, WIDTH-610*WIDTH/800, 460*HEIGHT/600, FONT_SIZE_1, 0, COLOR(255,20,20), "Odejít");

    sDisplay->PrintText(MAIN_FONT, 3,HEIGHT-12-5, FONT_SIZE_1, 0, NOCOLOR,"Verze %u.%u (%s)", 0, 1, __DATE__);
}

void MenuStage::OnKeyStateChange(uint16 key, bool press)
{
    if (key == VK_UP && press)
        sDisplay->SetAngleX(-1.0f, true);
}

void MenuStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
    // Nova hra
    if (IN_RANGE(x,y, WIDTH-300*WIDTH/800, WIDTH, 70*HEIGHT/600, (70*HEIGHT/600)+80))
    {
        sApplication->SetStage(STAGE_GAMESETTINGS, 100);
        return;
    }
    // Multiplayer
    else if (IN_RANGE(x,y, WIDTH-340*WIDTH/800, WIDTH, 180*HEIGHT/600, (180*HEIGHT/600)+80))
    {
        sNetwork->Connect(sConfig->HostName.c_str(), sConfig->NetworkPort);
        sApplication->SetStage(STAGE_GAMESETTINGS);
        return;
    }
    // Odejit
    else if (IN_RANGE(x,y, WIDTH-610*WIDTH/800, WIDTH-310, 460*HEIGHT/600, (460*HEIGHT/600)+80))
    {
        exit(0);
        return;
    }
    // Nastaveni
    else if (IN_RANGE(x,y, WIDTH-390*WIDTH/800, WIDTH, 285*HEIGHT/600, (285*HEIGHT/600)+80))
    {
        sApplication->SetStage(STAGE_OPTIONS);
        return;
    }
    // Credits
    else if (IN_RANGE(x,y, WIDTH-490*WIDTH/800, WIDTH-190, 380*HEIGHT/600, (380*HEIGHT/600)+80))
    {
        sApplication->SetStage(STAGE_CREDITS);
        return;
    }
}
