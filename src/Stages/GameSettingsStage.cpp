#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Config.h>
#include <Network.h>
#include <Helpers.h>
#include <Storage.h>
#include <Gameplay.h>
#include <Effects/Animations.h>

void GameSettingsStage::OnEnter()
{
    // Defaultni hodnoty nastaveni
    m_selected = 0;
    m_nickfield = "";
    m_nickselected = false;

    m_selectedMode = GAME_TYPE_SP_CLASSIC;
    m_selectedMapId = 1;

    glClearColor(0.05f, 0.2f, 0.0f, 0.0f);

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

void GameSettingsStage::OnLeave()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
}

void GameSettingsStage::OnDraw()
{
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    float mult = 0.5f;
    if (WIDTH > 800 && HEIGHT > 600)
        mult = 1.0f;
    else if (WIDTH > 640 && HEIGHT > 480)
        mult = 0.7f;

    sDisplay->Draw2D(1, WIDTH-(640*mult), HEIGHT-(624*mult), 640*mult, 624*mult);

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

    //////////////
    // Multiplayer
    if (m_subStage == 0)
    {
       sDisplay->PrintText(MAIN_FONT, 100, 100, FONT_SIZE_N, 0, NOCOLOR, "P�ipojov�n�...");

       if (sNetwork->IsConnected())
       {
           SmartPacket initiate(CMSG_INITIATE_SESSION);
           sNetwork->SendPacket(&initiate);
           m_subStage = 1;
       }
    }
    else if (m_subStage == 1)
    {
       sDisplay->PrintText(MAIN_FONT, 100, 100, FONT_SIZE_N, 0, NOCOLOR, "Inicializace komunikace...");
    }
    else if (m_subStage == 2)
    {
       sDisplay->PrintText(MAIN_FONT, 100, 100, FONT_SIZE_N, 0, NOCOLOR, "Ov��ov�n�...");
    }
    else if (m_subStage == 3)
    {
       sDisplay->PrintText(MAIN_FONT, 100, 100, FONT_SIZE_N, 0, NOCOLOR, "Z�sk�v�n� instance listu...");
    }
    else if (m_subStage == 4)
    {
        // Nick nadpis
        sDisplay->Draw2D(12, 20, 20, 500, 80);
        // Nick pole
        sDisplay->Draw2D(15, 30, 60, 500, 30);
        sDisplay->PrintText(MAIN_FONT, 35, 65, FONT_SIZE_N, 0, NOCOLOR, "%s%s", m_nickfield.c_str(), m_nickselected?" |":"");
        // Mistnosti nadpis
        sDisplay->Draw2D(13, 20, 130, 200, 40);
        // Mistnosti podklad
        sDisplay->Draw2D(15, 30, 180, WIDTH-60, HEIGHT-180-80);

        // Vykresleni mistnosti
        sDisplay->Draw2D(14, 5*WIDTHPCT, 190.0f+m_selected*27-2, 90*WIDTHPCT, 28);

        std::vector<string> exploded = explode(sStorage->m_instanceList.c_str(),'|');
        for (int i = 0; (i < exploded.size()/4) && (i < (18*sConfig->WindowHeight/768)); i++)
        {
            sDisplay->PrintText(MAIN_FONT, 5*WIDTHPCT+10, 194.0f+i*27, FONT_SIZE_N, 0, NOCOLOR,"%s",exploded[(i*4)+0].c_str());
            sDisplay->PrintText(MAIN_FONT, 10*WIDTHPCT  , 194.0f+i*27, FONT_SIZE_N, 0, NOCOLOR,"%s",exploded[(i*4)+1].c_str());
            sDisplay->PrintText(MAIN_FONT, 50*WIDTHPCT  , 194.0f+i*27, FONT_SIZE_N, 0, NOCOLOR,"%s",exploded[(i*4)+2].c_str());
            sDisplay->PrintText(MAIN_FONT, 70*WIDTHPCT  , 194.0f+i*27, FONT_SIZE_N, 0, NOCOLOR,"%s",exploded[(i*4)+3].c_str());
        }

        // Tlacitko "Pripojit"
        sDisplay->Draw2D(16, WIDTH-200, HEIGHT-60, 200, 40);
    }
    else if (m_subStage == 5)
    {
        sDisplay->PrintText(MAIN_FONT, 150, 150, FONT_SIZE_N, 0, NOCOLOR, "Z�sk�v�n� dat o instanci...");
    }
    ////////////
    // Singleplayer
    else if (m_subStage == 100)
    {
        // Vyber modu

        if (m_selectedMode != GAME_TYPE_SP_MIN)
            sDisplay->Draw2D(39, 60, 65, 50, 25);  // Sipka doleva

        if (m_selectedMode != GAME_TYPE_SP_MAX)
            sDisplay->Draw2D(37, 430, 65, 50, 25); // Sipka doprava

        switch (m_selectedMode)
        {
            case GAME_TYPE_SP_CLASSIC:
                sDisplay->PrintText(FONT_ONE, 170, 65, FONT_SIZE_3, 0, NOCOLOR, "Rychl� hra");

                // TODO: funkce pro odstavcovy text a zalamovani
                sDisplay->Draw2D(15, 80, 130, WIDTH-80-80, HEIGHT-130-150);
                sDisplay->PrintParagraphText(FONT_ONE, 100, 150, WIDTH-100-100, FONT_SIZE_4, 0, NOCOLOR,
                    "Rychl� hra je z�kladn�m typem hry. Princip je inspirov�n p�vodn� p�edlohou. Obsahuje klasick� krabice, bonusy, nep��tele. Podm�nkou v�hry je zdolat " \
                    "v�echny nep��tele v �asov�m limitu.");
                break;
            case GAME_TYPE_SP_MEME:
                sDisplay->PrintText(FONT_ONE, 170, 65, FONT_SIZE_3, 0, NOCOLOR, "Meme wars");

                // TODO: funkce pro odstavcovy text a zalamovani
                sDisplay->Draw2D(15, 80, 130, WIDTH-80-80, HEIGHT-130-150);
                sDisplay->PrintParagraphText(FONT_ONE, 100, 150, WIDTH-100-100, FONT_SIZE_4, 0, NOCOLOR,
                    "Meme wars je typ hry inspirovan� internetov�mi meme postavi�kami a " \
                    "srandi�kami, jako nap��klad troll, nyan cat, rickroll, rage comics, pedobear, " \
                    "o rly?, a dal��. �kolem je op�t zni�it v�echny nep��tele v �asov�m limitu, " \
                    "ov�em za pomoci bonus�, kter� se v origin�ln� h�e neobjevily. Co t�eba Nyan " \
                    "cat speed bonus?");

                break;
            default:
                break;
        }

        // Dalsi
        sDisplay->Draw2D(37, WIDTH-200, HEIGHT-120, 100, 50);
        // Zpet
        sDisplay->Draw2D(39, 100, HEIGHT-120, 100, 50);
    }
    else if (m_subStage == 101)
    {
        // Zalezi opet na modu, jaky jsme si vybrali
        switch (m_selectedMode)
        {
            case GAME_TYPE_SP_CLASSIC:
            {
                sDisplay->PrintText(FONT_ONE, WIDTH/2-64*4+4*36/2, 70, FONT_SIZE_N, 0, NOCOLOR, "Rychl� hra");

                sDisplay->PrintText(FONT_ONE, 50, 150, FONT_SIZE_4, 0, NOCOLOR, "Po�et nep��tel");
                sDisplay->PrintText(MAIN_FONT, 105, 183, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_ENEMY_COUNT));
                sDisplay->Draw2D(43, 80, 180, 18, 20);
                sDisplay->Draw2D(41, 140, 180, 18, 20);

                uint32 spacing = 0;
                uint32 leftspacing = 300;
                sDisplay->PrintText(FONT_ONE, 50+leftspacing, 150+spacing, FONT_SIZE_4, 0, NOCOLOR, "Intenzita beden");
                sDisplay->PrintText(MAIN_FONT, 105+leftspacing, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_BOX_DENSITY));
                sDisplay->Draw2D(43, 80+leftspacing, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 140+leftspacing, 180+spacing, 18, 20);

                spacing = 80;
                sDisplay->PrintText(FONT_ONE, 50, 150+spacing, FONT_SIZE_4, 0, NOCOLOR, "Inteligence nep��tel");
                sDisplay->PrintText(MAIN_FONT, 105, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_ENEMY_AI_LEVEL));
                sDisplay->Draw2D(43, 80, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 140, 180+spacing, 18, 20);

                sDisplay->PrintText(FONT_ONE, 50+leftspacing, 150+spacing, FONT_SIZE_3, 0, NOCOLOR, "Mapa");
                std::string mapname = "! Nezn�m� mapa !";
                if (sStorage->MapData.find(sGameplayMgr->GetSetting(SETTING_MAP_ID)) != sStorage->MapData.end())
                    mapname = sStorage->MapData[sGameplayMgr->GetSetting(SETTING_MAP_ID)].mapname;
                sDisplay->PrintText(MAIN_FONT, 105+leftspacing, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%s", mapname.c_str());
                sDisplay->Draw2D(43, 80+leftspacing, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 380+leftspacing, 180+spacing, 18, 20);

                spacing = 2*80;
                sDisplay->PrintText(FONT_ONE, 50, 150+spacing, FONT_SIZE_4, 0, NOCOLOR, "Rychlost nep��tel");
                sDisplay->PrintText(MAIN_FONT, 105, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_ENEMY_SPEED));
                sDisplay->Draw2D(43, 80, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 140, 180+spacing, 18, 20);
                break;
            }
            case GAME_TYPE_SP_MEME:
            {
                sDisplay->PrintText(FONT_ONE, WIDTH/2-64*4+4*36/2, 70, FONT_SIZE_N, 0, NOCOLOR, "Meme wars");

                sDisplay->PrintText(FONT_ONE, 50, 150, FONT_SIZE_4, 0, NOCOLOR, "Po�et nep��tel");
                sDisplay->PrintText(MAIN_FONT, 105, 183, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_ENEMY_COUNT));
                sDisplay->Draw2D(43, 80, 180, 18, 20);
                sDisplay->Draw2D(41, 140, 180, 18, 20);

                uint32 spacing = 0;
                uint32 leftspacing = 300;
                sDisplay->PrintText(FONT_ONE, 50+leftspacing, 150+spacing, FONT_SIZE_4, 0, NOCOLOR, "Intenzita beden");
                sDisplay->PrintText(MAIN_FONT, 105+leftspacing, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_BOX_DENSITY));
                sDisplay->Draw2D(43, 80+leftspacing, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 140+leftspacing, 180+spacing, 18, 20);

                spacing = 80;
                sDisplay->PrintText(FONT_ONE, 50, 150+spacing, FONT_SIZE_4, 0, NOCOLOR, "Inteligence nep��tel");
                sDisplay->PrintText(MAIN_FONT, 105, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_ENEMY_AI_LEVEL));
                sDisplay->Draw2D(43, 80, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 140, 180+spacing, 18, 20);

                sDisplay->PrintText(FONT_ONE, 50+leftspacing, 150+spacing, FONT_SIZE_3, 0, NOCOLOR, "Mapa");
                std::string mapname = "! Nezn�m� mapa !";
                if (sStorage->MapData.find(sGameplayMgr->GetSetting(SETTING_MAP_ID)) != sStorage->MapData.end())
                    mapname = sStorage->MapData[sGameplayMgr->GetSetting(SETTING_MAP_ID)].mapname;
                sDisplay->PrintText(MAIN_FONT, 105+leftspacing, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%s", mapname.c_str());
                sDisplay->Draw2D(43, 80+leftspacing, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 380+leftspacing, 180+spacing, 18, 20);

                spacing = 2*80;
                sDisplay->PrintText(FONT_ONE, 50, 150+spacing, FONT_SIZE_4, 0, NOCOLOR, "Rychlost nep��tel");
                sDisplay->PrintText(MAIN_FONT, 105, 183+spacing, FONT_SIZE_N, 0, NOCOLOR, "%u", sGameplayMgr->GetSetting(SETTING_ENEMY_SPEED));
                sDisplay->Draw2D(43, 80, 180+spacing, 18, 20);
                sDisplay->Draw2D(41, 140, 180+spacing, 18, 20);
                break;
            }
        }

        // Tlacitko "Hrat"
        sDisplay->Draw2D(20, WIDTH-200, HEIGHT-120, 200, 40);
        // Zpet
        sDisplay->Draw2D(39, 100, HEIGHT-120, 100, 50);
    }
}

void GameSettingsStage::OnKeyStateChange(uint16 key, bool press)
{
    if (m_nickselected && press)
    {
        unsigned char ckey = (unsigned char)key;
        if (!sApplication->IsKeyPressed(VK_SHIFT))
            ckey = tolower(ckey);

        // backspace
        if (key == 8)
        {
            if (m_nickfield.size() > 0)
                m_nickfield.erase(m_nickfield.size()-1);
        }
        else
        {
            // numericka klavesnice
            if (key >= 96 && key <= 105)
                ckey = '0'+(ckey-96);

            // Restrikce pro platne znaky a-z, A-Z, 0-9
            if ((ckey >= 65 && ckey <= 90) || (ckey >= 96 && ckey <= 122) || (ckey >= 48 && ckey <= 57))
            {
                char newcontent[256];
                sprintf(newcontent, "%s%c", m_nickfield.c_str(), ckey);
                m_nickfield = newcontent;
            }
        }
    }
}

void GameSettingsStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
    ////////////
    // Multiplayer
    if (m_subStage < 100)
    {
        // Kliknuti do pole pro nick
        if (x > 30 && x < 500+30 && y > 60 && y < 60+30)
            m_nickselected = true;
        else
            m_nickselected = false;

        // Kliknuti do pole s mistnostmi
        if (x > 30 && x < WIDTH-60+30 && y > 180 && y < HEIGHT-180-80+180)
        {
            uint32 pos = (y-190)/27;

            std::vector<string> exploded = explode(sStorage->m_instanceList.c_str(),'|');
            if ((exploded.size()/4) > pos)
                m_selected = pos;
        }

        // "Pripojit"
        if (x > WIDTH-200 && x < WIDTH && y > HEIGHT-60 && y < HEIGHT-20)
        {
            SmartPacket enter(CMSG_ENTER_GAME);
            enter << m_selected;
            enter << m_nickfield.c_str();
            sNetwork->SendPacket(&enter);

            sStorage->m_nickName = m_nickfield.c_str();
        }
    }
    ////////////
    // Singleplayer
    else if (m_subStage == 100)
    {
        // Vyber modu

        // Sipka doleva
        if (x > 60 && y > 65 && x < 60+50 && y < 65+25)
        {
            // Posun jen pokud jsme na jinem nez prvnim modu
            if (m_selectedMode != GAME_TYPE_SP_MIN)
                m_selectedMode = GameType(uint32(m_selectedMode)-1);
        }
        // Sipka doprava
        else if (x > 430 && y > 65 && x < 430+50 && y < 65+25)
        {
            // Posun jen pokud jsme na mensim nez je maximalni mod
            if (m_selectedMode != GAME_TYPE_SP_MAX)
                m_selectedMode = GameType(uint32(m_selectedMode)+1);
        }
        // Sipka pro pokracovani
        else if (x > WIDTH-200 && x < WIDTH-100 && y > HEIGHT-120 && y < HEIGHT-70)
        {
            // Dalsi staz - vyber podrobnosti
            m_subStage = 101;
        }
        // Sipka pro vraceni
        else if (x > 100 && x < 200 && y > HEIGHT-120 && y < HEIGHT-70)
        {
            sApplication->SetStage(STAGE_MENU);
            return;
        }
    }
    else if (m_subStage == 101)
    {
        // Pocet nepratel
        // doleva
        if (IN_RANGE(x,y,80,80+18,180,180+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_ENEMY_COUNT) > 1)
                 sGameplayMgr->SetSetting(SETTING_ENEMY_COUNT, sGameplayMgr->GetSetting(SETTING_ENEMY_COUNT) - 1);
        }
        // doprava
        else if (IN_RANGE(x,y,140,140+18,180,180+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_ENEMY_COUNT) < 15)
                 sGameplayMgr->SetSetting(SETTING_ENEMY_COUNT, sGameplayMgr->GetSetting(SETTING_ENEMY_COUNT) + 1);
        }
        // Intenzita beden
        // doleva
        if (IN_RANGE(x,y,380,380+18,180,180+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_BOX_DENSITY) > 1)
                sGameplayMgr->SetSetting(SETTING_BOX_DENSITY, sGameplayMgr->GetSetting(SETTING_BOX_DENSITY) - 1);
        }
        // doprava
        else if (IN_RANGE(x,y,440,440+18,180,180+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_BOX_DENSITY) < 10)
                 sGameplayMgr->SetSetting(SETTING_BOX_DENSITY, sGameplayMgr->GetSetting(SETTING_BOX_DENSITY) + 1);
        }
        // MAPA
        // doleva
        if (IN_RANGE(x,y,380,380+18,260,260+20))
        {
            // begin() vraci platny iterator na prvni prvek v mape, kdezto end() vraci neplatny pointer
            // na "jeden prvek za poslednim" v mape, proto mirna odlisnost kodu pro inkrementaci a dekrementaci
            Storage::MapDataMap::const_iterator itr = sStorage->MapData.find(sGameplayMgr->GetSetting(SETTING_MAP_ID));
            if (itr != sStorage->MapData.begin())
                sGameplayMgr->SetSetting(SETTING_MAP_ID, (--itr)->first);
        }
        // doprava
        else if (IN_RANGE(x,y,680,680+18,260,260+20))
        {
            Storage::MapDataMap::const_iterator itr = sStorage->MapData.find(sGameplayMgr->GetSetting(SETTING_MAP_ID));
            if (++itr != sStorage->MapData.end())
                sGameplayMgr->SetSetting(SETTING_MAP_ID, itr->first);
        }
        // AI level nepratel
        // doleva
        if (IN_RANGE(x,y,80,80+18,260,260+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_ENEMY_AI_LEVEL) > 1)
                 sGameplayMgr->SetSetting(SETTING_ENEMY_AI_LEVEL, sGameplayMgr->GetSetting(SETTING_ENEMY_AI_LEVEL) - 1);
        }
        // doprava
        else if (IN_RANGE(x,y,140,140+18,260,260+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_ENEMY_AI_LEVEL) < 3)
                 sGameplayMgr->SetSetting(SETTING_ENEMY_AI_LEVEL, sGameplayMgr->GetSetting(SETTING_ENEMY_AI_LEVEL) + 1);
        }
        // Rychlost nepratel
        // doleva
        if (IN_RANGE(x,y,80,80+18,340,340+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_ENEMY_SPEED) > 1)
                 sGameplayMgr->SetSetting(SETTING_ENEMY_SPEED, sGameplayMgr->GetSetting(SETTING_ENEMY_SPEED) - 1);
        }
        // doprava
        else if (IN_RANGE(x,y,140,140+18,340,340+20))
        {
            if (sGameplayMgr->GetSetting(SETTING_ENEMY_SPEED) < 5)
                 sGameplayMgr->SetSetting(SETTING_ENEMY_SPEED, sGameplayMgr->GetSetting(SETTING_ENEMY_SPEED) + 1);
        }
        // "Hrat"
        else if (x > WIDTH-200 && x < WIDTH && y > HEIGHT-120 && y < HEIGHT-80)
        {
            sGameplayMgr->SetGameType(m_selectedMode);
            sApplication->SetStage(STAGE_LOADING, 1);
        }
        // Sipka pro vraceni
        else if (x > 100 && x < 200 && y > HEIGHT-120 && y < HEIGHT-70)
        {
            m_subStage = 100;
            return;
        }
    }
}
