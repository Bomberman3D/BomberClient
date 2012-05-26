#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Config.h>

static const uint32 resolutionMap[][3] = {
    {0, 640, 480},
    {1, 800, 600},
    {2, 1024, 768},
    {3, 1152, 768},
    {4, 1280, 800},
    {5, 1280, 960},
    {6, 1280, 1024},
    {7, 1366, 768},
    {8, 1600, 1200},
    {9, 1680, 1050},
    {10, 1920, 1080}
};

static const uint32 colorDepthMap[][2] = {
    {0, 16},
    {1, 32}
};

static const uint32 refreshRateMap[][2] = {
    {0, 30},
    {1, 60},
    {2, 75},
    {3, 100}
};

void OptionsStage::OnEnter()
{
    m_selResolution = 0;
    for (uint32 i = 0; i < sizeof(resolutionMap)/sizeof(uint32[3]); i++)
    {
        if (resolutionMap[i][1] == sConfig->WindowWidth && resolutionMap[i][2] == sConfig->WindowHeight)
        {
            m_selResolution = i;
            break;
        }
    }

    m_selColorDepth = 0;
    for (uint32 i = 0; i < sizeof(colorDepthMap)/sizeof(uint32[2]); i++)
    {
        if (colorDepthMap[i][1] == sConfig->ColorDepth)
        {
            m_selColorDepth = i;
            break;
        }
    }

    m_selRefreshRate = 0;
    for (uint32 i = 0; i < sizeof(refreshRateMap)/sizeof(uint32[2]); i++)
    {
        if (refreshRateMap[i][1] == sConfig->RefreshRate)
        {
            m_selRefreshRate = i;
            break;
        }
    }

    m_fullscreen = sConfig->fullscreen;
}

void OptionsStage::OnDraw(uint32 diff)
{
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    // Ramecky
    sDisplay->Draw2D(76, 30, 30, 30, 30);
    sDisplay->Draw2D(77, WIDTH-30-30, 30, 30, 30);
    sDisplay->Draw2D(78, WIDTH-30-30, HEIGHT-30-30, 30, 30);
    sDisplay->Draw2D(79, 30, HEIGHT-30-30, 30, 30);

    // -2 a +4 magie kvuli 1 pixelovemu paddingu na kraji kazde textury, ktery osobne nechapu
    // vyresim to jindy, momentalne je to takova jedna mala vada na krase no..
    sDisplay->Draw2D(72, 30, 30+30-2, 30, HEIGHT-30-30-30-30+4);
    sDisplay->Draw2D(73, 30+30-2, 30, WIDTH-30-30-30-30+4, 30);
    sDisplay->Draw2D(74, WIDTH-30-30, 30+30-2, 30, HEIGHT-30-30-30-30+4);
    sDisplay->Draw2D(75, 30+30-2, HEIGHT-30-30, WIDTH-30-30-30-30+4, 30);

    sDisplay->PrintText(FONT_ONE, 100, 140, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Rozlišení obrazovky");
    sDisplay->PrintText(FONT_ONE, 130, 175, FONT_SIZE_3, 0, NOCOLOR, "%u x %u", resolutionMap[m_selResolution][1], resolutionMap[m_selResolution][2]);
    if (m_selResolution > 0)
        sDisplay->Draw2D(43, 110, 175, 12, 24);
    if (m_selResolution < sizeof(resolutionMap)/sizeof(uint32[3])-1)
        sDisplay->Draw2D(41, 410, 175, 12, 24);

    sDisplay->PrintText(FONT_ONE, 100, 140+100, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Barevná hloubka");
    sDisplay->PrintText(FONT_ONE, 130, 175+100, FONT_SIZE_3, 0, NOCOLOR, "%u bit", colorDepthMap[m_selColorDepth][1]);
    if (m_selColorDepth > 0)
        sDisplay->Draw2D(43, 110, 175+100, 12, 24);
    if (m_selColorDepth < sizeof(colorDepthMap)/sizeof(uint32[2])-1)
        sDisplay->Draw2D(41, 410, 175+100, 12, 24);

    sDisplay->PrintText(FONT_ONE, 100, 140+200, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Celá obrazovka");
    sDisplay->PrintText(FONT_ONE, 130, 175+200, FONT_SIZE_3, 0, NOCOLOR, "%s", m_fullscreen?"Ano":"Ne");
    if (m_fullscreen)
        sDisplay->Draw2D(43, 110, 175+200, 12, 24);
    else
        sDisplay->Draw2D(41, 410, 175+200, 12, 24);

    sDisplay->PrintText(FONT_ONE, 100, 140+300, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Obnovovací frekvence");
    sDisplay->PrintText(FONT_ONE, 130, 175+300, FONT_SIZE_3, 0, NOCOLOR, "%u Hz", refreshRateMap[m_selRefreshRate][1]);
    if (m_selRefreshRate > 0)
        sDisplay->Draw2D(43, 110, 175+300, 12, 24);
    if (m_selRefreshRate < sizeof(refreshRateMap)/sizeof(uint32[2])-1)
        sDisplay->Draw2D(41, 410, 175+300, 12, 24);

    sDisplay->PrintText(FONT_ONE, WIDTH-424, HEIGHT-118, FONT_SIZE_3, 0, NOCOLOR, "Uložit");
    sDisplay->PrintText(FONT_ONE, WIDTH-224, HEIGHT-118, FONT_SIZE_3, 0, NOCOLOR, "Zpìt");
}

void OptionsStage::OnKeyStateChange(uint16 key, bool press)
{
}

void OptionsStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
    // Vyradime ze hry prave tlacitko mysi
    if (!left)
        return;

    // Rozliseni
    if (IN_RANGE(x, y, 110, 110+12, 175, 175+24))
    {
        if (m_selResolution > 0)
            m_selResolution--;
    }
    else if (IN_RANGE(x, y, 410, 410+12, 175, 175+24))
    {
        if (m_selResolution < sizeof(resolutionMap)/sizeof(uint32[3])-1)
            m_selResolution++;
    }

    // Barevna hloubka
    if (IN_RANGE(x, y, 110, 110+12, 175+100, 175+100+24))
    {
        if (m_selColorDepth > 0)
            m_selColorDepth--;
    }
    else if (IN_RANGE(x, y, 410, 410+12, 175+100, 175+100+24))
    {
        if (m_selColorDepth < sizeof(colorDepthMap)/sizeof(uint32[2])-1)
            m_selColorDepth++;
    }

    // Plna obrazovka
    if (IN_RANGE(x, y, 110, 110+12, 175+200, 175+200+24))
    {
        if (m_fullscreen)
            m_fullscreen = false;
    }
    else if (IN_RANGE(x, y, 410, 410+12, 175+200, 175+200+24))
    {
        if (!m_fullscreen)
            m_fullscreen = true;
    }

    // Obnovovaci frekvence
    if (IN_RANGE(x, y, 110, 110+12, 175+300, 175+300+24))
    {
        if (m_selRefreshRate > 0)
            m_selRefreshRate--;
    }
    else if (IN_RANGE(x, y, 410, 410+12, 175+300, 175+300+24))
    {
        if (m_selRefreshRate < sizeof(refreshRateMap)/sizeof(uint32[2])-1)
            m_selRefreshRate++;
    }

    // Ulozit
    if (IN_RANGE(x, y, WIDTH-424, WIDTH-264, HEIGHT-118, HEIGHT-90))
    {
        sConfig->WindowWidth   = resolutionMap[m_selResolution][1];
        sConfig->WindowHeight  = resolutionMap[m_selResolution][2];
        sConfig->ColorDepth    = colorDepthMap[m_selColorDepth][1];
        sConfig->fullscreen    = m_fullscreen;
        sConfig->RefreshRate   = refreshRateMap[m_selRefreshRate][1];

        sConfig->Save();
        //sApplication->SetStage(STAGE_MENU);
        // Fuck that shit, i'm gonna be Leeroy Jenkins when I grow up
        sApplication->PMessageBox("Upozornìní", "Ke zmìnì vlastností okna je tøeba restartovat hru. Nyní bude aplikace ukonèena.");
        exit(0);
    }
    // Zpet
    if (IN_RANGE(x, y, WIDTH-224, WIDTH-64, HEIGHT-118, HEIGHT-90))
    {
        sApplication->SetStage(STAGE_MENU);
    }
}
