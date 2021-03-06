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
#include <SoundMgr.h>
#include <Network.h>

void GameStage::OnEnter()
{
    sSoundMgr->InitMusicPlaylist();
    sSoundMgr->MusicStart();
    m_subStage = 0;
    m_consoleHistoryLine = 0;

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
    sSoundMgr->MusicStop();
    sGameplayMgr->OnGameLeave();

    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
}

void GameStage::OnBeforeDraw()
{
    sDisplay->DrawMap();
}

void GameStage::OnDraw()
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

    if (sGameplayMgr->GetDamageScreenTime()+1000 >= clock())
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        float alpha = float(clock()-sGameplayMgr->GetDamageScreenTime()) / 1000.0f;
        sDisplay->Draw2DColor(0xFF000000 | uint8(96 * (1.0f-alpha)), 0, 0, WIDTH, HEIGHT);
    }

    // Hrajeme!
    if (m_subStage == 0)
    {
        // Stitky se jmenem nad enemakama
        if (sGameplayMgr->gameFeatures.enemyNamePlates)
        {
            EnemyList* enemies = sGameplayMgr->GetEnemies();
            if (enemies && !enemies->empty())
            {
                sDisplay->Setup3DMode();
                glDisable(GL_LIGHTING);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                for (EnemyList::const_iterator itr = enemies->begin(); itr != enemies->end(); ++itr)
                {
                    glLoadIdentity();
                    sDisplay->AdjustViewToTarget(true);

                    glTranslatef((*itr)->pRecord->x, (*itr)->pRecord->y+1.5f, (*itr)->pRecord->z);
                    glRotatef(90.0f-sDisplay->GetAngleY(), 0.0f, 1.0f, 0.0f);

                    sDisplay->BindTexture(50);

                    const char* name = (*itr)->GetName();
                    float width = (float)strlen(name) * 0.018f * 10 * FONT_SIZE_2;

                    glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2f(1.0f, 1.0f); glVertex3f(0, 0,    -width/2-0.09f);
                        glTexCoord2f(0.0f, 1.0f); glVertex3f(0, 0,     width/2+0.09f);
                        glTexCoord2f(1.0f, 0.0f); glVertex3f(0, 0.2f, -width/2-0.09f);
                        glTexCoord2f(0.0f, 0.0f); glVertex3f(0, 0.2f,  width/2+0.09f);
                    glEnd();

                    if (sGameplayMgr->gameFeatures.maxEnemyHealth > 0)
                    {
                        // 0.9f je maximalni sirka HP pole, 0.8f kvuli oriznuti 0.05f na kazde strane pro HP bar (ramecek)
                        float hpwidth = 0.8f * ((float)(*itr)->GetHealth()/(float)(*itr)->GetMaxHealth());
                        glBegin(GL_TRIANGLE_STRIP);
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(0,  0.0f, -0.45f);
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(0,  0.0f,  0.45f);
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(0, -0.2f, -0.45f);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(0, -0.2f,  0.45f);
                        glEnd();
                        sDisplay->BindTexture(49);
                        glTranslatef(-0.01f, 0, 0);
                        glBegin(GL_TRIANGLE_STRIP);
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(0, -0.05f, -hpwidth/2);
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(0, -0.05f,  hpwidth/2);
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(0, -0.15f, -hpwidth/2);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(0, -0.15f,  hpwidth/2);
                        glEnd();
                    }
                    else
                        glTranslatef(-0.01f, 0, 0);

                    glTranslatef(0, 0.18f, (-width/2) - 0.05f);
                    sDisplay->PrintText(MAIN_FONT, 0, 0, FONT_SIZE_2, TEXT_FLAG_3D, NOCOLOR, name);
                }

                glEnable(GL_LIGHTING);
                sDisplay->Setup2DMode();
            }
        }

        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        // Vytiskneme, co prave bezi za hudbu
        if (sSoundMgr->GetLastMusicStart() > 0 && sSoundMgr->GetLastMusicStart() > clock()-4000)
        {
            int32 music = sSoundMgr->GetCurrentMusicId();
            if (music > 0)
            {
                sDisplay->Draw2D(15, WIDTH-300, 20, 300, 80);
                sDisplay->PrintText(FONT_ONE, WIDTH-300+10, 25, 0.275f, 0, COLOR(255, 127, 127), "Music:");
                sDisplay->PrintText(FONT_ONE, WIDTH-300+10, 50, 0.3f, 0, NOCOLOR, "%s",sStorage->MusicData[uint32(music)].author.c_str());
                sDisplay->PrintText(FONT_ONE, WIDTH-300+10, 76, 0.3f, 0, NOCOLOR, "%s",sStorage->MusicData[uint32(music)].description.c_str());
            }
        }

        if (sGameplayMgr->gameFeatures.miniMap)
        {
            // Vykreslime minimapu do leveho horniho rohu
            Map* pMap = (Map*)sMapManager->GetMap();
            uint32 textureId = 0;
            if (pMap && sGameplayMgr->GetPlayerRec())
            {
                uint32 bx = floor(sGameplayMgr->GetPlayerRec()->x)+1;
                uint32 by = floor(sGameplayMgr->GetPlayerRec()->z)+1;

                uint32 mapsize_x = pMap->field.size();
                uint32 mapsize_y = pMap->field[0].size();

                EnemyList* enemies = sGameplayMgr->GetEnemies();

                // Mapa bude mit vzdy (maximalne) 160 obrazovych bodu dlouhou jednu "hranu"
                // rozhodne se podle delsiho rozmeru
                // 8/20 je pomer, puvodne tedy "na mapu o sirce 20 poli bude 8 pixelove vykreslene pole"
                float field_size = 0.0f;
                if (mapsize_x < mapsize_y)
                    field_size = mapsize_x*8/20;
                else
                    field_size = mapsize_y*8/20;

                // Preklad na stred mapy a pootoceni tak, aby byla vzdy vuci hraci "rovne"
                // cili aby smer "nahoru" na 2D ose fungoval jako smer "dopredu" v prostoru
                glTranslatef(20+(mapsize_x*field_size)/2, 20+(mapsize_y*field_size)/2, 0.0f);
                glRotatef(180.0f+sGameplayMgr->GetPlayerRec()->rotate-sDisplay->GetHorizontalAngleDeviation(), 0,0,1);
                glTranslatef(-int32(20+(mapsize_x*field_size)/2), -int32(20+(mapsize_y*field_size)/2), 0.0f);

                for (uint32 i = 0; i < mapsize_x; i++)
                {
                    for (uint32 j = 0; j < mapsize_y; j++)
                    {
                        // Nastaveni textury pro vykresleni
                        if (pMap->IsDynamicRecordPresent(i, j, DYNAMIC_TYPE_BOX))
                            textureId = 52;
                        //else if (sGameplayMgr->WouldBeDangerousField(i,j))
                        //    textureId = 51;
                        else
                        {
                            switch (pMap->field[i][j].type)
                            {
                                case TYPE_SOLID_BOX:
                                    textureId = 50;
                                    break;
                                case TYPE_GROUND:
                                default:
                                    textureId = 49;
                                    break;
                            }
                        }
                        sDisplay->Draw2D(textureId, 20+i*field_size, 20+j*field_size, field_size, field_size);

                        if (!sGameplayMgr->IsSingleGameType() && pMap->IsDynamicRecordPresent(i, j, DYNAMIC_TYPE_BONUS))
                            sDisplay->Draw2D(55, 20+i*field_size, 20+j*field_size, field_size, field_size);
                    }
                }

                // Vykreslit hraci puntik
                sDisplay->Draw2D(53, 20+bx*field_size, 20+by*field_size, field_size, field_size);

                // Vykreslit kazdemu hraci v multiplayeru puntik
                if (!sGameplayMgr->IsSingleGameType() && sNetwork->IsConnected())
                {
                    for (PlayerList::const_iterator itr = sNetwork->players.begin(); itr != sNetwork->players.end(); ++itr)
                    {
                        if ((*itr) && (*itr)->rec)
                        {
                            bx = floor((*itr)->rec->x)+1;
                            by = floor((*itr)->rec->z)+1;

                            sDisplay->Draw2D(54, 20+bx*field_size, 20+by*field_size, field_size, field_size);
                        }
                    }
                }

                // A taky puntik pro vsechny nepratele :)
                if (enemies && !enemies->empty())
                {
                    for (EnemyList::iterator itr = enemies->begin(); itr != enemies->end(); ++itr)
                        sDisplay->Draw2D(54, 20+ceil((*itr)->pRecord->x)*field_size, 20+ceil((*itr)->pRecord->z)*field_size, field_size, field_size);
                }
            }
        }

        glLoadIdentity();

        if (sGameplayMgr->gameFeatures.bonusCounter)
        {
            sDisplay->Draw2D(33, 240, 40 , 24, 24); // plamen
            sDisplay->PrintText(MAIN_FONT, 290, 42, FONT_SIZE_H2, 0, COLOR(0,0,127), "%ux", (sGameplayMgr->GetFlameReach()-1));

            sDisplay->Draw2D(34, 240, 90 , 24, 24); // rychlost
            sDisplay->PrintText(MAIN_FONT, 290, 92, FONT_SIZE_H2, 0, COLOR(0,0,127), "%ux", uint32((sGameplayMgr->GetPlayerSpeedCoef()-1.0f)/0.1f));

            sDisplay->Draw2D(35, 240, 140, 24, 24); // bomby
            sDisplay->PrintText(MAIN_FONT, 290, 142, FONT_SIZE_H2, 0, COLOR(0,0,127), "%ux", (sGameplayMgr->GetMaxBombs()-1));
        }

        if (sGameplayMgr->gameFeatures.gameTimer)
        {
            clock_t endtime = sGameplayMgr->GetGameEndTime();
            if (endtime != 0)
            {
                if (endtime > clock())
                {
                    uint32 min = (endtime-clock()) / 60000;
                    uint32 sec = ((endtime-clock()) / 1000) - min * 60;
                    sDisplay->PrintText(MAIN_FONT, 403, 52, FONT_SIZE_H3, 0, COLOR(0,0,127), "%u:%02u", min, sec);
                }
                else
                    sDisplay->PrintText(MAIN_FONT, 403, 52, FONT_SIZE_H3, 0, COLOR(0,0,127), "0:00");
            }
        }

        if (sGameplayMgr->gameFeatures.maxHealth > 0)
        {
            sDisplay->Draw2D(50, 390, 120, 100, 30);

            uint32 health = sGameplayMgr->GetHealth();
            uint32 sz = health;
            float space = 0;
            while (sz > 0)
            {
                space += 0.5f;
                sz /= 10;
            }

            float mult = (float)health/(float)sGameplayMgr->gameFeatures.maxHealth;

            sDisplay->Draw2D(((mult > 0.2f) ? 49 : 51), 395, 125, 90*mult, 20);
            sDisplay->PrintText(MAIN_FONT, 440-space*10*FONT_SIZE_1-2, 129, FONT_SIZE_1, 0, NOCOLOR, "%u", health);
        }

        // Skore
        if (sApplication->IsKeyPressed(VK_TAB))
        {
            sDisplay->Draw2D(15, 150, 120, WIDTH-150-150, 320);

            uint32 mover = 0;
            sDisplay->PrintText(MAIN_FONT, 160, 130, 1.0f, 0,     COLOR(255,127,127), "Nick");
            sDisplay->PrintText(MAIN_FONT, 160+200*WIDTH/1024.0f, 130, 1.0f, 0, COLOR(255,127,127), "Po�et zabit�");
            sDisplay->PrintText(MAIN_FONT, 160+350*WIDTH/1024.0f, 130, 1.0f, 0, COLOR(255,127,127), "Po�et smrt�");
            for (ScoreBoard::const_iterator itr = sStorage->m_scoreBoard.begin(); itr != sStorage->m_scoreBoard.end(); ++itr)
            {
                sDisplay->PrintText(MAIN_FONT, 160, 160+mover*20, 1.0f, 0, NOCOLOR, "%s", (*itr).nickName.c_str(), (*itr).kills, (*itr).deaths);
                sDisplay->PrintText(MAIN_FONT, 160+200*WIDTH/1024.0f, 160+mover*20, 1.0f, 0, NOCOLOR, "%u", (*itr).kills);
                sDisplay->PrintText(MAIN_FONT, 160+350*WIDTH/1024.0f, 160+mover*20, 1.0f, 0, NOCOLOR, "%u", (*itr).deaths);
                mover++;
            }
        }
    }
    // Hra zapauzovana
    else if (m_subStage == 2)
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        sDisplay->Draw2D(15, 150, 80, WIDTH-150-150, 320);
        sDisplay->PrintText(FONT_ONE, WIDTH/2-38*4.5f*FONT_SIZE_1, 130, FONT_SIZE_1, 0, NOCOLOR, "P a u z a");

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*5.5f*FONT_SIZE_3, WIDTH/2+38*5.5f*FONT_SIZE_3, 250-5, 250+35))
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250    , FONT_SIZE_3, 0, COLOR(200, 200, 0), "Zp�t do hry");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250    , FONT_SIZE_3, 0, COLOR(200, 0, 0), "Zp�t do hry");

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*5.5f*FONT_SIZE_3,  WIDTH/2+38*5.5f*FONT_SIZE_3, 250+35, 250+95))
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250+60 , FONT_SIZE_3, 0, COLOR(200, 200, 0), "Ukon�it hru");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*FONT_SIZE_3, 250+60 , FONT_SIZE_3, 0, COLOR(200, 0, 0), "Ukon�it hru");
    }
    // Game over!
    else if (m_subStage == 3)
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        // 38 px na 1 znak
        sDisplay->Draw2D(14, WIDTH/8, 2*HEIGHT/7, 3*WIDTH/4, 100);
        sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5, 2*HEIGHT/7+25, FONT_SIZE_N, 0, COLOR(255, 0, 0), "Jsi mrtev!");

        if (sGameplayMgr->IsSingleGameType())
        {
            sDisplay->Draw2D(15, WIDTH/4, 2*HEIGHT/7+100, WIDTH/2, 160);

            if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*6*0.6f, WIDTH/2+38*6*0.6f, 2*HEIGHT/7+125, 2*HEIGHT/7+125+60))
                sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5*0.6f, 2*HEIGHT/7+125, FONT_SIZE_2, 0, COLOR(255, 255, 0)  , "Pokra�ovat");
            else
                sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5*0.6f, 2*HEIGHT/7+125, FONT_SIZE_2, 0, COLOR(255, 0, 255), "Pokra�ovat");

            if (IN_RANGE(mousePos.x, mousePos.y, WIDTH/2-38*6*0.6f, WIDTH/2+38*6*0.6f, 2*HEIGHT/7+125+60, 2*HEIGHT/7+125+130))
                sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*0.6f, 2*HEIGHT/7+125+70, FONT_SIZE_2, 0, COLOR(255, 255, 0)  , "Hlavn� menu");
            else
                sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5.5f*0.6f, 2*HEIGHT/7+125+70, FONT_SIZE_2, 0, COLOR(255, 0, 255), "Hlavn� menu");
        }
        else
        {
            sDisplay->Draw2D(15, WIDTH/4, 2*HEIGHT/7+100, WIDTH/2, 100);
            if (sStorage->m_respawnTime)
            {
                int32 delta = (int32)((int64)sStorage->m_respawnTime-(int64)time(NULL));
                if (delta < 0)
                {
                    delta = 0;
                    if (!sStorage->m_respawnRequest)
                    {
                        sGameplayMgr->SendRespawnRequest();
                        sStorage->m_respawnRequest = true;
                    }
                }
                if (!sStorage->m_respawnRequest)
                    sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5*0.6f, 2*HEIGHT/7+125, FONT_SIZE_3, 0, COLOR(255, 255, 0)  , "O�iven� za %u...", delta);
                else
                    sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5*0.6f, 2*HEIGHT/7+125, FONT_SIZE_3, 0, COLOR(255, 255, 0)  , "�ek�m na o�iven�...", delta);
            }
        }
    }
    // Statistiky (po smrti)
    else if (m_subStage == 4 || m_subStage == 5)
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        sDisplay->Draw2D(15, 0, 0, WIDTH, HEIGHT);

        // Singleplayer only
        // TODO: multiplayer stats (vice hracu, ...)

        if (m_subStage == 4)
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*5, 50, FONT_SIZE_N, 0, COLOR(255,0,0), "Statistiky");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH/2-38*3, 50, FONT_SIZE_N, 0, NOCOLOR, "V�hra!");

        sDisplay->PrintText(FONT_ONE, 1*HEIGHT/8, 150, FONT_SIZE_4, 0, COLOR(255,255,255), PlayerStatsNames[sGameplayMgr->GetGameType()][0]);
        sDisplay->PrintText(MAIN_FONT, 1*HEIGHT/8+20, 150+50, FONT_SIZE_H3, 0, COLOR(255,255,255), "%u", sGameplayMgr->localPlayerStats.UniversalStats.field1);
        sDisplay->PrintText(FONT_ONE, 3*HEIGHT/8, 150, FONT_SIZE_4, 0, COLOR(255,255,255), PlayerStatsNames[sGameplayMgr->GetGameType()][1]);
        sDisplay->PrintText(MAIN_FONT, 3*HEIGHT/8+20, 150+50, FONT_SIZE_H3, 0, COLOR(255,255,255), "%u", sGameplayMgr->localPlayerStats.UniversalStats.field2);
        sDisplay->PrintText(FONT_ONE, 5*HEIGHT/8, 150, FONT_SIZE_4, 0, COLOR(255,255,255), PlayerStatsNames[sGameplayMgr->GetGameType()][2]);
        sDisplay->PrintText(MAIN_FONT, 5*HEIGHT/8+20, 150+50, FONT_SIZE_H3, 0, COLOR(255,255,255), "%u", sGameplayMgr->localPlayerStats.UniversalStats.field3);

        if (IN_RANGE(mousePos.x, mousePos.y, 50, 50+38*12, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            sDisplay->PrintText(FONT_ONE, 50,                         HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, 0, COLOR(255, 255, 0), "Zkusit znovu");
        else
            sDisplay->PrintText(FONT_ONE, 50,                         HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, 0, COLOR(255, 0, 0), "Zkusit znovu");

        if (IN_RANGE(mousePos.x, mousePos.y, WIDTH-50-38*12*FONT_SIZE_2, WIDTH-50, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            sDisplay->PrintText(FONT_ONE, WIDTH-50-38*12*FONT_SIZE_2, HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, 0, COLOR(255, 255, 0), "Zp�t do menu");
        else
            sDisplay->PrintText(FONT_ONE, WIDTH-50-38*12*FONT_SIZE_2, HEIGHT-64*FONT_SIZE_2-50, FONT_SIZE_2, 0, COLOR(255, 0, 0), "Zp�t do menu");
    }

    // Vykresleni konzole
    if (sGameplayMgr->IsSingleGameType())
    {
        if (sGameplayMgr->IsConsoleOpened())
        {
            if (!sDisplay->IsIn2DMode())
                sDisplay->Setup2DMode();

            // Pozadi
            sDisplay->Draw2D(14, 5, 0, WIDTH-10, 140);

            // Historie vystupu
            for (uint32 i = 0; i < CONSOLE_OUTPUT_LINES; i++)
                sDisplay->PrintText(MAIN_FONT, 10, 5+i*20, 1.0f, 0, NOCOLOR, "%s", sGameplayMgr->GetConsoleOutput(i));

            // Aktualni vstup
            sDisplay->PrintText(MAIN_FONT, 10, 5+CONSOLE_OUTPUT_LINES*20+15, 1.0f, 0, COLOR(255,200,200), "> %s", sGameplayMgr->GetConsoleInput());
        }
    }
    else // V multiplayeru vyuzivame konzoli jako chat, a zobrazujeme ji stale
    {
        if (m_subStage == 0)
        {
            if (!sDisplay->IsIn2DMode())
                sDisplay->Setup2DMode();

            // Pozadi
            if (sGameplayMgr->IsConsoleOpened())
                sDisplay->Draw2D(80, 0, HEIGHT-90, WIDTH/2, 90);

            // Historie vystupu
            for (uint32 i = 0; i < CONSOLE_OUTPUT_LINES; i++)
                sDisplay->PrintText(MAIN_FONT, 5, HEIGHT - 6*14 + i*14, 0.7f, 0, NOCOLOR, "%s", sGameplayMgr->GetConsoleOutput(i));

            // Aktualni vstup
            if (sGameplayMgr->IsConsoleOpened())
                sDisplay->PrintText(MAIN_FONT, 5, HEIGHT-14, 0.7f, 0, COLOR(255,200,200), "> %s", sGameplayMgr->GetConsoleInput());
        }
    }

    sDisplay->Setup3DMode();
}

bool IsAllowedChatKey(unsigned char key)
{
    static uint8 addAllowed[] = {'.',',',':','?','!','_','-','/','\\','"','\'',')','('};

    if ((key >= 'a' && key <= 'z')
        || (key >= 'A' && key <= 'Z')
        || (key >= '0' && key <= '9')
        || key == VK_SPACE)
        return true;

    for (uint32 i = 0; i < sizeof(addAllowed)/sizeof(uint8); i++)
        if (key == addAllowed[i])
            return true;

    return false;
}

void GameStage::OnPrintableChar(uint16 key)
{
    if (sGameplayMgr->IsConsoleOpened())
    {
        // tisknutelne znaky
        if (IsAllowedChatKey(key))
        {
            if (!sGameplayMgr->IsSingleGameType())
            {
                if ((key >= 'a' && key <= 'z') && sApplication->IsKeyPressed(VK_SHIFT))
                    key = (key-'a')+'A';
                if ((key >= 'A' && key <= 'Z') && !sApplication->IsKeyPressed(VK_SHIFT))
                    key = (key-'A')+'a';
            }

            char* newline = new char[strlen(sGameplayMgr->GetConsoleInput())+1];
            sprintf(newline, "%s%c", sGameplayMgr->GetConsoleInput(), key);
            sGameplayMgr->SetConsoleInput(newline);
            m_consoleHistoryLine = 0;
        }
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
            if (sGameplayMgr->IsConsoleOpened())
                sGameplayMgr->CloseConsole();
            else
            {
                sGameplayMgr->UnpauseGame();
                m_subStage = 0;
            }
        }
        else if (m_subStage == 0)
        {
            sGameplayMgr->PauseGame();
            m_subStage = 2;
        }
    }

    // Pokud je substage rovna 2 (menu), muzeme otevrit konzoli
    if (m_subStage == 2)
    {
        if (key == 192 && press)
        {
            if (sGameplayMgr->IsConsoleOpened())
                sGameplayMgr->CloseConsole();
            else
            {
                sGameplayMgr->OpenConsole();
                return;
            }
        }
    }

    // v multiplayeru konzole = chat, a otevira se enterem
    if (m_subStage == 0)
    {
        if (key == VK_RETURN && press)
        {
            if (!sGameplayMgr->IsConsoleOpened())
            {
                sGameplayMgr->OpenConsole();
                return;
            }
        }
    }

    // Psani do konzole
    if (sGameplayMgr->IsConsoleOpened() && press)
    {
        // backspace
        if (key == VK_BACK)
        {
            char* newline = new char[strlen(sGameplayMgr->GetConsoleInput())];
            sprintf(newline, "%s", sGameplayMgr->GetConsoleInput());
            newline[strlen(newline)-1] = '\0';
            sGameplayMgr->SetConsoleInput(newline);
            m_consoleHistoryLine = 0;
        }
        // sipka nahoru - historie
        else if (key == VK_UP && sGameplayMgr->IsSingleGameType())
        {
            if (m_consoleHistoryLine < CONSOLE_INPUT_HISTORY)
            {
                m_consoleHistoryLine++;
                sGameplayMgr->SetConsoleInput(sGameplayMgr->GetConsoleInputHistory(CONSOLE_INPUT_HISTORY-m_consoleHistoryLine));
            }
        }
        // sipka dolu - historie
        else if (key == VK_DOWN && sGameplayMgr->IsSingleGameType())
        {
            if (m_consoleHistoryLine > 0)
            {
                m_consoleHistoryLine--;
                sGameplayMgr->SetConsoleInput(sGameplayMgr->GetConsoleInputHistory(CONSOLE_INPUT_HISTORY-m_consoleHistoryLine));
            }
        }
        // enter - potvrzeni
        else if (key == VK_RETURN)
        {
            sGameplayMgr->ConsoleSubmit();
            m_consoleHistoryLine = 0;
        }
    }

    // Pokud je substage rovna 2 (menu) nebo je otevrena konzole, zadny pohyb !
    if (m_subStage == 2 || sGameplayMgr->IsConsoleOpened())
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
        if (m_subStage == 0)
        {
            Map* pMap = (Map*)sMapManager->GetMap();
            if (!pMap)
                return;

            if (m_lastBombPlant + 300 > clock())
                return;

            m_lastBombPlant = clock();

            uint32 bx = floor(sGameplayMgr->GetPlayerRec()->x)+1;
            uint32 by = floor(sGameplayMgr->GetPlayerRec()->z)+1;

            if (sGameplayMgr->IsSingleGameType())
            {
                if (sGameplayMgr->AddBomb(bx, by))
                {
                    pMap->AddDynamicCell(bx, by, DYNAMIC_TYPE_BOMB, 0, 0, NULL);
                    sMapManager->FillDynamicRecords();
                    sDisplay->m_ignoreTargetCollision = DYNAMIC_TYPE_BOMB;
                }
            }
            else
            {
                // jen pozadat server o vlozeni bomby
                sGameplayMgr->SendAddBomb(bx, by);
            }
        }
        // jsme zapauzovani
        else if (m_subStage == 2)
        {
            // Zpet do hry
            if (IN_RANGE(x, y, WIDTH/2-38*5.5f*FONT_SIZE_3, WIDTH/2+38*5.5f*FONT_SIZE_3, 250-5, 250+35))
            {
                sGameplayMgr->CloseConsole();
                sGameplayMgr->UnpauseGame();
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
            // Zp�t do menu
            else if (IN_RANGE(x, y, WIDTH/2-38*6*0.6f, WIDTH/2+38*5.5f*0.6f, 2*HEIGHT/7+125+60, 2*HEIGHT/7+125+130))
            {
                sApplication->SetStage(STAGE_MENU);
            }
        }
        // jsme ve statistikach
        else if (m_subStage == 4 || m_subStage == 5)
        {
            if (IN_RANGE(x, y, 50, 50+38*12, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            {
                sGameplayMgr->UnpauseGame();
                sApplication->SetStage(STAGE_LOADING, 1);
            }
            else if (IN_RANGE(x, y, WIDTH-50-38*12*FONT_SIZE_2, WIDTH-50, HEIGHT-64*FONT_SIZE_2-50, HEIGHT-50))
            {
                sGameplayMgr->UnpauseGame();
                sApplication->SetStage(STAGE_MENU);
            }
        }
    }
}

void GameStage::OnUpdate()
{
    // Odsud budeme updatovat i GameplayMgr
    // Nejdrive uhel podle mysi
    sGameplayMgr->UpdatePlayerMoveAngle();
    // Pak pohyb
    sGameplayMgr->UpdatePlayerMotion();
    // A pote zbytek
    sGameplayMgr->Update();
}
