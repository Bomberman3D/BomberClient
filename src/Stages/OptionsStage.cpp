#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Config.h>

static const uint32 resolutionMap[][2] = {
    //{640, 480}, // Toho, komu nezbyde nic jineho, nez spustit hru na tomto rozliseni, velice lituji a nabizim k zapujceni muj stary Intel i486 PC
    {800, 600},
    {1024, 600},
    {1024, 768},
    {1152, 768},
    {1280, 800},
    {1280, 960},
    {1280, 1024},
    {1366, 768},
    {1600, 1200},
    {1680, 1050},
    {1920, 1080}
};

static const uint32 colorDepthMap[] = {
    16,
    32
};

static const uint32 refreshRateMap[] = {
    30,
    60,
    75,
    100
};

void OptionsStage::OnEnter()
{
    m_selResolution = 0;
    for (uint32 i = 0; i < sizeof(resolutionMap)/sizeof(uint32[2]); i++)
    {
        if (resolutionMap[i][0] == sConfig->WindowWidth && resolutionMap[i][1] == sConfig->WindowHeight)
        {
            m_selResolution = i;
            break;
        }
    }

    m_selColorDepth = 0;
    for (uint32 i = 0; i < sizeof(colorDepthMap)/sizeof(uint32); i++)
    {
        if (colorDepthMap[i] == sConfig->ColorDepth)
        {
            m_selColorDepth = i;
            break;
        }
    }

    m_selRefreshRate = 0;
    for (uint32 i = 0; i < sizeof(refreshRateMap)/sizeof(uint32); i++)
    {
        if (refreshRateMap[i] == sConfig->RefreshRate)
        {
            m_selRefreshRate = i;
            break;
        }
    }

    m_selMusicVolume = sConfig->MusicVolume/5;
    m_selEffectVolume = sConfig->EffectVolume/5;

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
    sDisplay->PrintText(FONT_ONE, 130, 175, FONT_SIZE_3, 0, NOCOLOR, "%u x %u", resolutionMap[m_selResolution][0], resolutionMap[m_selResolution][1]);
    if (m_selResolution > 0)
        sDisplay->Draw2D(43, 110, 175, 12, 24);
    if (m_selResolution < sizeof(resolutionMap)/sizeof(uint32[2])-1)
        sDisplay->Draw2D(41, 410, 175, 12, 24);

    sDisplay->PrintText(FONT_ONE, 100, 140+100, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Barevná hloubka");
    sDisplay->PrintText(FONT_ONE, 130, 175+100, FONT_SIZE_3, 0, NOCOLOR, "%u bit", colorDepthMap[m_selColorDepth]);
    if (m_selColorDepth > 0)
        sDisplay->Draw2D(43, 110, 175+100, 12, 24);
    if (m_selColorDepth < sizeof(colorDepthMap)/sizeof(uint32)-1)
        sDisplay->Draw2D(41, 410, 175+100, 12, 24);

    sDisplay->PrintText(FONT_ONE, 100, 140+200, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Celá obrazovka");
    sDisplay->PrintText(FONT_ONE, 130, 175+200, FONT_SIZE_3, 0, NOCOLOR, "%s", m_fullscreen?"Ano":"Ne");
    if (m_fullscreen)
        sDisplay->Draw2D(43, 110, 175+200, 12, 24);
    else
        sDisplay->Draw2D(41, 410, 175+200, 12, 24);

    sDisplay->PrintText(FONT_ONE, 100, 140+300, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Obnovovací frekvence");
    sDisplay->PrintText(FONT_ONE, 130, 175+300, FONT_SIZE_3, 0, NOCOLOR, "%u Hz", refreshRateMap[m_selRefreshRate]);
    if (m_selRefreshRate > 0)
        sDisplay->Draw2D(43, 110, 175+300, 12, 24);
    if (m_selRefreshRate < sizeof(refreshRateMap)/sizeof(uint32)-1)
        sDisplay->Draw2D(41, 410, 175+300, 12, 24);

    sDisplay->PrintText(FONT_ONE, WIDTH-424, HEIGHT-118, FONT_SIZE_3, 0, NOCOLOR, "Uložit");
    sDisplay->PrintText(FONT_ONE, WIDTH-224, HEIGHT-118, FONT_SIZE_3, 0, NOCOLOR, "Zpìt");

    //////////////////////////////
    // Druhy sloupec

    sDisplay->PrintText(FONT_ONE, 450, 140, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Hlasitost hudby [%%]");
    sDisplay->PrintText(FONT_ONE, 480, 175, FONT_SIZE_3, 0, NOCOLOR, "%u%%", m_selMusicVolume*5);
    if (m_selMusicVolume > 0)
        sDisplay->Draw2D(43, 460, 175, 12, 24);
    if (m_selMusicVolume < 20)
        sDisplay->Draw2D(41, 610, 175, 12, 24);

    sDisplay->PrintText(FONT_ONE, 450, 140+100, FONT_SIZE_3-0.05f, 0, COLOR(255,127,255), "Hlasitost efektù [%%]");
    sDisplay->PrintText(FONT_ONE, 480, 175+100, FONT_SIZE_3, 0, NOCOLOR, "%u%%", m_selEffectVolume*5);
    if (m_selEffectVolume > 0)
        sDisplay->Draw2D(43, 460, 175+100, 12, 24);
    if (m_selEffectVolume < 20)
        sDisplay->Draw2D(41, 610, 175+100, 12, 24);
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
        if (m_selResolution < sizeof(resolutionMap)/sizeof(uint32[2])-1)
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
        if (m_selColorDepth < sizeof(colorDepthMap)/sizeof(uint32)-1)
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
        if (m_selRefreshRate < sizeof(refreshRateMap)/sizeof(uint32)-1)
            m_selRefreshRate++;
    }

    // Hlasitost hudby
    if (IN_RANGE(x, y, 460, 460+12, 175, 175+24))
    {
        if (m_selMusicVolume > 0)
            m_selMusicVolume--;
    }
    else if (IN_RANGE(x, y, 610, 610+12, 175, 175+24))
    {
        if (m_selMusicVolume < 20)
            m_selMusicVolume++;
    }

    // Hlasitost efektu
    if (IN_RANGE(x, y, 460, 460+12, 175+100, 175+100+24))
    {
        if (m_selEffectVolume > 0)
            m_selEffectVolume--;
    }
    else if (IN_RANGE(x, y, 610, 610+12, 175+100, 175+100+24))
    {
        if (m_selEffectVolume < 20)
            m_selEffectVolume++;
    }

    // Ulozit
    if (IN_RANGE(x, y, WIDTH-424, WIDTH-264, HEIGHT-118, HEIGHT-90))
    {
        sConfig->WindowWidth   = resolutionMap[m_selResolution][0];
        sConfig->WindowHeight  = resolutionMap[m_selResolution][1];
        sConfig->ColorDepth    = colorDepthMap[m_selColorDepth];
        sConfig->fullscreen    = m_fullscreen;
        sConfig->RefreshRate   = refreshRateMap[m_selRefreshRate];
        sConfig->MusicVolume   = m_selMusicVolume*5;
        sConfig->EffectVolume  = m_selEffectVolume*5;

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
