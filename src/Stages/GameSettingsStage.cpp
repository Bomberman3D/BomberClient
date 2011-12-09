#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Config.h>
#include <Network.h>
#include <Helpers.h>

void GameSettingsStage::OnEnter()
{
    // Menu scene
    sDisplay->DrawModel(4, -0.5f, -5.0f, -8.0f, ANIM_NONE, 2.0f);
    sDisplay->SetAngleX(20.0f);

    m_selected = 0;
    m_nickfield = "";
    m_nickselected = false;
}

void GameSettingsStage::OnDraw(uint32 diff)
{
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    //////////////
    // Multiplayer
    if (m_subStage == 0)
    {
       sDisplay->PrintText(MAIN_FONT, 100,100, "Pøipojování...");

       if (sNetwork->IsConnected())
       {
           SmartPacket initiate(CMSG_INITIATE_SESSION);
           sNetwork->SendPacket(&initiate);
           m_subStage = 1;
       }
    }
    else if (m_subStage == 1)
    {
       sDisplay->PrintText(MAIN_FONT, 100,100, "Inicializace komunikace...");
    }
    else if (m_subStage == 2)
    {
       sDisplay->PrintText(MAIN_FONT, 100,100, "Ovìøování...");
    }
    else if (m_subStage == 3)
    {
       sDisplay->PrintText(MAIN_FONT, 100,100, "Získávání instance listu...");
    }
    else if (m_subStage == 4)
    {
        // Nick nadpis
        sDisplay->Draw2D(12, 20, 20, 500, 80);
        // Nick pole
        sDisplay->Draw2D(15, 30, 60, 500, 30);
        sDisplay->PrintText(MAIN_FONT, 35, 65, "%s%s", m_nickfield.c_str(), m_nickselected?" |":"");
        // Mistnosti nadpis
        sDisplay->Draw2D(13, 20, 130, 200, 40);
        // Mistnosti podklad
        sDisplay->Draw2D(15, 30, 180, WIDTH-60, HEIGHT-180-80);

        // Vykresleni mistnosti
        sDisplay->Draw2D(14, 5*WIDTHPCT, 190.0f+m_selected*27-2, 90*WIDTHPCT, 28);

        std::vector<string> exploded = explode(sStorage->m_instanceList.c_str(),'|');
        for (int i = 0; (i < exploded.size()/4) && (i < (18*sConfig->WindowHeight/768)); i++)
        {
            sDisplay->PrintText(MAIN_FONT, 5*WIDTHPCT+10, 194.0f+i*27,"%s",exploded[(i*4)+0].c_str());
            sDisplay->PrintText(MAIN_FONT, 10*WIDTHPCT  , 194.0f+i*27,"%s",exploded[(i*4)+1].c_str());
            sDisplay->PrintText(MAIN_FONT, 50*WIDTHPCT  , 194.0f+i*27,"%s",exploded[(i*4)+2].c_str());
            sDisplay->PrintText(MAIN_FONT, 70*WIDTHPCT  , 194.0f+i*27,"%s",exploded[(i*4)+3].c_str());
        }

        // Tlacitko "Pripojit"
        sDisplay->Draw2D(16, WIDTH-200, HEIGHT-60, 200, 40);
    }
    ////////////
    // Singleplayer
    else if (m_subStage == 100)
    {
        // Tlacitko "Hrat"
        sDisplay->Draw2D(20, WIDTH-200, HEIGHT-60, 200, 40);
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
            //
        }
    }
    ////////////
    // Singleplayer
    else
    {
        // "Hrat"
        if (x > WIDTH-200 && x < WIDTH && y > HEIGHT-60 && y < HEIGHT-20)
        {
            sApplication->SetStage(STAGE_LOADING, 1);
        }
    }
}
