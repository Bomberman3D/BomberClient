/**********************************
*        Bomberman3D Engine       *
*  Created by: Cmaranec (Kennny)  *
**********************************/
// Main unit

#include <Global.h>
#include <Application.h>
#include <Storage.h>
#include <Config.h>
#include <Display.h>
#include <Effects/ParticleEmitter.h>
#include <Network.h>
#include <Timer.h>
#include <LoadingThread.h>
#include <SoundMgr.h>
#include <ScriptMgr.h>

#include <io.h>
#include <stdlib.h>
#include <fcntl.h>

#include <GL/glut.h>

HDC hDC       = NULL;
HGLRC hRC     = NULL;
HGLRC hRC_sec = NULL;
HWND hWnd     = NULL;
HINSTANCE hInstance;

bool fullscreen = false;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
    if (height == 0)
    {
        height = 1;
    }

    sConfig->WindowWidth = width;
    sConfig->WindowHeight = height;

    glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/** \brief Inicializace OpenGL rozhrani
 *
 * Zde se pouze zinicializuje veskere zobrazeni
 */
bool InitGL(GLvoid)
{
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glutInitDisplayMode(GLUT_DEPTH);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);

    sDisplay->StartUp();
    sDisplay->Initialize();

    return true;
}

/** \brief Hlavni updatovaci funkce aplikace
 *
 * Zde se zpracovava veskery update vsech trid a subsystemu.
 * Tato funkce se vola pri kazdem pruchodu cyklu v Application::Run
 */
void Application::Update()
{
    if (sDisplay->IsIn2DMode())
        sDisplay->Setup3DMode();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    ProcessInterThreadRequests();

    m_currStage->OnBeforeDraw();
    sDisplay->Update();
    m_currStage->OnDraw();
    m_currStage->OnUpdate();
    sSoundMgr->Update();
    sTimer->Update();

    // Kurzor - vykresluje se vzdycky, krome samotne herni faze pri hrani (ne pauza)
    if (m_currStage->GetType() != STAGE_GAME || m_currStage->GetSubStage() != 0)
    {
        if (!sDisplay->IsIn2DMode())
            sDisplay->Setup2DMode();

        sDisplay->Draw2D(4, GetMouseX(), GetMouseY(), 30, 37);
    }

    m_lastUpdate = clock();
}

void KillGLWindow(GLvoid)
{
    if (fullscreen)
    {
        ChangeDisplaySettings(NULL,0);
        ShowCursor(TRUE);
    }

    if (hRC)
    {
        if (!wglMakeCurrent(NULL,NULL))
        {
            MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }

        if (!wglDeleteContext(hRC))
        {
            MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }
        hRC = NULL;
    }

    // Vyradime z provozu nacitaci vlakno - nutnost pockat na ukonceni kvuli rendering contextu a jeho vycisteni
    time_t fuck_that_shit = time(NULL) + 3;
    sLoader->ShutdownThread();
    while (!sLoader->m_isDead)
    {
        if (time(NULL) >= fuck_that_shit)
            break;
    }

    if (hRC_sec)
    {
        if (!wglDeleteContext(hRC_sec))
        {
            MessageBox(NULL,"Release Second Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        }
        hRC_sec = NULL;
    }

    if (hDC && !ReleaseDC(hWnd,hDC))
    {
        MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hDC = NULL;
    }

    if (hWnd && !DestroyWindow(hWnd))
    {
        MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hWnd = NULL;
    }

    if (!UnregisterClass("OpenGL",hInstance))
    {
        MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;
    }
}

bool CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag, int refreshrate)
{
    GLuint PixelFormat;
    WNDCLASS wc;
    DWORD dwExStyle;
    DWORD dwStyle;
    RECT WindowRect;
    WindowRect.left   = (long)0;
    WindowRect.right  = (long)width;
    WindowRect.top    = (long)0;
    WindowRect.bottom = (long)height;

    fullscreen = fullscreenflag;

    hInstance          = GetModuleHandle(NULL);
    wc.style           = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc     = (WNDPROC) WndProc;
    wc.cbClsExtra      = 0;
    wc.cbWndExtra      = 0;
    wc.hInstance       = hInstance;
    wc.hIcon           = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground   = NULL;
    wc.lpszMenuName    = NULL;
    wc.lpszClassName   = "OpenGL";

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (fullscreen)
    {
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
        dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth  = width;
        dmScreenSettings.dmPelsHeight = height;
        dmScreenSettings.dmBitsPerPel = bits;
        dmScreenSettings.dmFields     = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
        dmScreenSettings.dmDisplayFrequency = refreshrate;

        if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!= DISP_CHANGE_SUCCESSFUL)
        {
            if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","ERROR",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
            {
                fullscreen = FALSE;
            }
            else
            {
                MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
                return false;
            }
        }
    }

    if (fullscreen)
    {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

    if (!(hWnd = CreateWindowEx(dwExStyle,
                                "OpenGL",
                                title,
                                dwStyle |
                                WS_CLIPSIBLINGS |
                                WS_CLIPCHILDREN,
                                0, 0,
                                WindowRect.right-WindowRect.left,
                                WindowRect.bottom-WindowRect.top,
                                NULL,
                                NULL,
                                hInstance,
                                NULL)))
    {
        KillGLWindow();
        MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    static PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        bits,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    if (!(hDC = GetDC(hWnd)))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!(PixelFormat = ChoosePixelFormat(hDC,&pfd)))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!SetPixelFormat(hDC,PixelFormat,&pfd))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!(hRC = wglCreateContext(hDC)))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!(hRC_sec = wglCreateContext(hDC)))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Create A Second GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!wglShareLists(hRC, hRC_sec))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't share display lists between main and loading rendering contexts","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!wglMakeCurrent(hDC,hRC))
    {
        KillGLWindow();
        MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    ShowWindow(hWnd,SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    ReSizeGLScene(width, height);

    if (!InitGL())
    {
        KillGLWindow();
        MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                return 0;
            }
            break;
        }
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_KEYDOWN:
        {
            sApplication->KeyStateChange(wParam, true);
            return 0;
        }
        case WM_KEYUP:
        {
            sApplication->KeyStateChange(wParam, false);
            return 0;
        }
        case WM_CHAR:
        {
            sApplication->PrintableCharPress(wParam);
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            sApplication->MouseButtonStateChange(true, true);
            return 0;
        }
        case WM_LBUTTONUP:
        {
            sApplication->MouseButtonStateChange(true, false);
            return 0;
        }
        /*case WM_MBUTTONDOWN:
        {
            return 0;
        }
        case WM_MBUTTONUP:
        {
            return 0;
        }*/
        case WM_RBUTTONDOWN:
        {
            sApplication->MouseButtonStateChange(false, true);
            return 0;
        }
        case WM_RBUTTONUP:
        {
            sApplication->MouseButtonStateChange(false, false);
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            sApplication->SetMouseXY(LOWORD(lParam), HIWORD(lParam));
            return 0;
        }
        case WM_SIZE:
        {
            ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
            return 0;
        }
        case WM_ACTIVATE:
        {
            if (wParam == WA_INACTIVE)
                sApplication->SetAppActive(false);
            else
                sApplication->SetAppActive(true);
            return 0;
        }
    }

    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (!sApplication->Init())
        return 0;

    return sApplication->Run();
}

/** \brief Konstruktor
 *
 * Zde pouze nulujeme veskera nastaveni
 */
Application::Application()
{
    m_currStage = NULL;
    m_lastUpdate = clock();
    console = false;

    for (uint16 i = 0; i < 256; i++)
        keys[i] = false;

    for (uint8 i = 0; i < 2; i++)
        mouse[i] = 0;

    for (uint8 i = 0; i < 2; i++)
        mouseXY[i] = 0;
}

/** \brief Destruktor
 *
 * Prazdny, netreba nic uvolnovat
 */
Application::~Application()
{
}

/** \brief Inicializacni rutina aplikace
 *
 * Zde se zinicializuji veskere handlery, vytvori se okno a nastavi se implicitni faze hry na Intro.
 * Je take mozne odkomentovat kod pro inicializaci testovaci konzole, ktera ve se Windows normalne nespousti.
 * To jen Linux nam casto automaticky doprava zapnutou konzoli se stdout vystupem
 */
bool Application::Init()
{
    srand((unsigned int)time(NULL));

    if (!sConfig->Load())
    {
        PMessageBox("Chyba", "Nelze na��st configy!");
        return false;
    }

    if (!CreateGLWindow("BomberEngine",sConfig->WindowWidth,sConfig->WindowHeight,sConfig->ColorDepth,sConfig->fullscreen,sConfig->RefreshRate))
        return false;

    ShowCursor(FALSE);

    sStorage->Load();

    sNetwork->Initialize();
    if (sNetwork->IsInitialized())
        boost::thread NetworkThread(runNetworkWorker);

    sApplication->ApplyRenderContext(RC_MAIN);

    sSoundMgr->Initialize();

    sScriptMgr->Initialize();

    m_currStage = new MenuStage;
    SetStage(STAGE_INTRO, 0); // Nasavit stage intro s parametrem 0, coz znaci uvodni prvotni intro

    return true;
}

/** \brief Hlavni funkce aplikace
 *
 * Tady se aplikace "toci" po cely svuj beh. Vola se zde update funkce, ktera se stara o herni mechanizmy,
 * zpracovavaji se tu zpravy okna a tak dale.
 */
int Application::Run()
{
    MSG msg;
    bool done = false;

    while (!done)
    {
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                done = true;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            Update();
            SwapBuffers(hDC);
        }
    }

    m_lastUpdate = clock();

    KillGLWindow();
    return msg.wParam;
}

/** \brief Zobrazi okynko s textem podle formatu (asi Windows only)
 *
 * Klasicky MessageBox, pouze s parametrovym stylem zadavani textu, jako ma napriklad printf funkce
 */
void Application::PMessageBox(const char* caption, const char* format, ...)
{
    va_list argList;
    va_start(argList,format);
    char buf[2048];
    vsnprintf(buf,2048,format,argList);
    va_end(argList);

    MessageBox(NULL, buf, caption, MB_OK);
}

/** \brief Hlavni prepinaci funkce pro stages hry
 *
 * Nastavi herni fazi podle zadaneho parametru \a newstage s para\a newphase
 */
void Application::SetStage(uint32 newstage, uint32 newphase, bool cleanup)
{
    if (m_currStage)
    {
        m_currStage->OnLeave();
        delete m_currStage;
    }

    switch (newstage)
    {
        case STAGE_INTRO:
            m_currStage = new IntroOutroStage;
            break;
        case STAGE_MENU:
            m_currStage = new MenuStage;
            break;
        case STAGE_GAMESETTINGS:
            m_currStage = new GameSettingsStage;
            break;
        case STAGE_OPTIONS:
            m_currStage = new OptionsStage;
            break;
        case STAGE_LOADING:
            m_currStage = new LoadingStage;
            break;
        case STAGE_GAME:
            m_currStage = new GameStage;
            break;
        case STAGE_CREDITS:
            m_currStage = new CreditsStage;
            break;
        default:
            assert(false && "Invalid stage!");
    }

    assert(m_currStage != NULL);

    if (cleanup)
    {
        sDisplay->FlushModelDisplayList();
        sDisplay->FlushBillboardDisplayList();
        sParticleEmitterMgr->FlushEmitters();
    }

    m_currStage->SetSubStage(newphase);
    m_currStage->OnEnter();
}

/** \brief Funkce pro zaznamenani zmeny stavu klavesy
 *
 * Tahle funkce je volana po zaznamenani prislusne akce handlerem zprav okna.
 * Vola i funkci herni faze pro zmenu stavu klavesy
 */
void Application::KeyStateChange(uint8 key, bool press)
{
    if (keys[key] != press)
    {
        keys[key] = press;

        m_currStage->OnKeyStateChange(key, press);

        /* Nezavisle na fazi hry muzeme vykonavat i nejake specialni funkce, platne pro jakoukoliv fazi.
         * Prikladem je treba alokovani / dealokovani konzole
         */

        // Zapnuti / vypnuti konzole
        // (CTRL + Y)
        // Pouze pro Windows !!
#ifdef WINVER
        if (key == 'Y' && keys[VK_CONTROL] && press)
        {
            if (console)
            {
                FreeConsole();
            }
            else
            {
                AllocConsole();

                // Presmerujeme handle stdoutu tak, abychom do nej mohli psat funkci fprintf
                int hCrt, i;
                FILE *hf;
                hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
                hf   = _fdopen(hCrt, "w");
                *stdout = *hf;
                i = setvbuf(stdout, NULL, _IONBF, 0);
            }

            console = !console;
        }
#endif
    }
}

/** \brief Funkce volana po stisknuti tisknutelneho znaku
 *
 * Oddelene od hlavniho key press/release kodu proto, ze prichazi rovnou jako znak od OS, cili
 * podleha veskeremu zvolenemu nastaveni jako znakova sada, locales a podobne
 */
void Application::PrintableCharPress(uint16 chr)
{
    if (m_currStage)
        m_currStage->OnPrintableChar(chr);
}

/** \brief Funkce pro zaznamenani zmeny stavu tlacitek mysi
 *
 * Tahle funkce je volana po zaznamenani zmeny stavu leveho nebo praveho tlacitka
 * Do stage handleru posila jen pokud bylo tlacitko stisknuto
 */
void Application::MouseButtonStateChange(bool left, bool press)
{
    uint8 pos = (left ? 0 : 1);
    if (mouse[pos] != press)
    {
        mouse[pos] = press;

        if (press)
        {
            m_currStage->OnMouseButtonPress(mouseXY[0], mouseXY[1], left);
            fprintf(stdout, "%s tlacitko mysi stisknuto [%u ; %u]\n", left?"Leve":"Prave", mouseXY[0], mouseXY[1]);
        }
    }
}

/** \brief Funkce pro nastaveni argumentu herni faze
 *
 * Pouze nastavi argument, prebirani uz je na samotnem stage handleru
 */
void Application::SetStagePhase(uint32 newphase)
{
    m_currStage->SetSubStage(newphase);
}

/** \brief Zmena render contextu
 *
 * Tato funkce se stara o zmenu rendering contextu pro dovoleni renderovani ve vice vlaknech najednou
 */
void Application::ApplyRenderContext(RenderingContext cont)
{
    switch (cont)
    {
        case RC_MAIN:
            wglMakeCurrent(hDC, hRC);
            break;
        case RC_LOADING:
            wglMakeCurrent(hDC, hRC_sec);
            break;
        default:
            PMessageBox("RC swich error", "Unhandled rendering context: %u", uint32(cont));
            break;
    }
}

/** \brief Zpracovani vsech mezivlaknovych pozadavku
 *
 * Tato funkce se stara o zpracovani pozadavku, ktere nemohou byt zpracovany pomoci jineho vlakna, nez hlavniho
 */
void Application::ProcessInterThreadRequests()
{
    sLockMgr->NeedToken(LOCK_STORAGE, THREAD_MAIN);

    while (!sLockMgr->HasToken(LOCK_STORAGE, THREAD_MAIN))
        boost::this_thread::yield();

    if (!sStorage->m_interThreadRequests.empty())
    {
        for (std::list<std::pair<uint32, uint32>>::iterator itr = sStorage->m_interThreadRequests.begin(); itr != sStorage->m_interThreadRequests.end(); )
        {
            switch (itr->first)
            {
                case REQUEST_STAGE_CHANGE:
                    sApplication->SetStage(itr->second);
                    break;
                case REQUEST_SUBSTAGE_CHANGE:
                    sApplication->SetStagePhase(itr->second);
                    break;
                case REQUEST_MAP_CHANGE:
                    sGameplayMgr->SetSetting(SETTING_MAP_ID, itr->second);
                    // TODO: another stuff?
                    break;
                case REQUEST_GAME_TYPE_CHANGE:
                    sGameplayMgr->SetGameType(GameType(itr->second));
                    break;
            }
            itr = sStorage->m_interThreadRequests.erase(itr);
        }
    }

    if (!sStorage->m_interThreadObjectRequests.empty())
    {
        for (std::list<std::pair<uint32, void*>>::iterator itr = sStorage->m_interThreadObjectRequests.begin(); itr != sStorage->m_interThreadObjectRequests.end(); )
        {
            if (itr->second != NULL || itr->first == REQUEST_DYNAMIC_MAP_FILL)
            {
                switch (itr->first)
                {
                    case REQUEST_DYNAMIC_MAP_ELEMENT:
                    {
                        ThreadRequestDynamicElement* tmp = (ThreadRequestDynamicElement*)(itr->second);
                        sMapManager->GetMap()->AddDynamicCell(tmp->x, tmp->y, tmp->rec.type, tmp->rec.state, tmp->rec.misc, NULL);
                        break;
                    }
                    case REQUEST_DYNAMIC_MAP_FILL:
                    {
                        // Musi byt handlovano zde, aby se zamezilo tomu, ze se to zavola driv, nez plneni dynamickych zaznamu
                        sMapManager->FillDynamicRecords();
                        break;
                    }
                    case REQUEST_PLAYER_ADD:
                    {
                        Player* pl = (Player*)(itr->second);
                        pl->rec = sDisplay->DrawModel(9, pl->x, 0.0f, pl->y, ANIM_IDLE, 3.5f, 1.0f, true, false, 0, 0, ANIM_RESTRICTION_NONE, false, pl->artkit);
                        sNetwork->players.push_back(pl);
                        break;
                    }
                    case REQUEST_PLAYER_ANIM:
                    {
                        ThreadRequestPlayerAnim* tmp = (ThreadRequestPlayerAnim*)(itr->second);
                        Player* pl = sNetwork->GetPlayerById(tmp->id);
                        if (!pl)
                            break;

                        sAnimator->ChangeModelAnim(pl->rec->AnimTicket, tmp->anim, 0, 0, tmp->flags);
                        break;
                    }
                    case REQUEST_PLAYER_POS:
                    {
                        ThreadRequestPlayerPos* tmp = (ThreadRequestPlayerPos*)(itr->second);
                        Player* pl = sNetwork->GetPlayerById(tmp->id);
                        if (!pl)
                            break;

                        pl->rec->x = tmp->x;
                        pl->rec->y = tmp->y;
                        pl->rec->z = tmp->z;
                        break;
                    }
                    case REQUEST_PLAYER_ROTATION:
                    {
                        ThreadRequestPlayerRotation* tmp = (ThreadRequestPlayerRotation*)(itr->second);
                        Player* pl = sNetwork->GetPlayerById(tmp->id);
                        if (!pl)
                            break;

                        pl->rec->rotate = tmp->rotation;
                        break;
                    }
                    case REQUEST_PLANT_BOMB:
                    {
                        ThreadRequestPlantBomb* tmp = (ThreadRequestPlantBomb*)(itr->second);
                        if (sGameplayMgr->AddBomb(tmp->x, tmp->y, tmp->id, tmp->reach))
                        {
                            Map* pMap = sMapManager->GetMap();

                            pMap->AddDynamicCell(tmp->x, tmp->y, DYNAMIC_TYPE_BOMB, 0, 0, NULL);
                            sMapManager->FillDynamicRecords();
                            if (tmp->id == sStorage->m_myId)
                                sDisplay->m_ignoreTargetCollision = DYNAMIC_TYPE_BOMB;
                        }
                        break;
                    }
                    case REQUEST_PLAYER_DEATH:
                    {
                        ThreadRequestPlayerDeath* tmp = (ThreadRequestPlayerDeath*)(itr->second);

                        if (tmp->id == sStorage->m_myId)
                        {
                            sGameplayMgr->PlayerDied();
                            if (tmp->respawnDelay)
                                sStorage->m_respawnTime = time(NULL) + tmp->respawnDelay;
                            else
                                sStorage->m_respawnTime = 0;

                            sStorage->m_respawnRequest = false;
                        }
                        else
                        {
                            Player* pl = sNetwork->GetPlayerById(tmp->id);
                            if (!pl)
                                break;

                            sAnimator->ChangeModelAnim(pl->rec->AnimTicket, ANIM_DYING, 0, 0, ANIM_FLAG_NOT_REPEAT);
                        }
                        break;
                    }
                    case REQUEST_BOX_DESTROY:
                    {
                        ThreadRequestBoxDestroy* tmp = (ThreadRequestBoxDestroy*)(itr->second);
                        Map* map = sMapManager->GetMap();
                        if (!map)
                            break;

                        map->DestroyDynamicRecords(tmp->x, tmp->y, DYNAMIC_TYPE_BOX);
                        break;
                    }
                    case REQUEST_FILL_SCOREBOARD:
                    {
                        ThreadRequestFillScoreBoard* tmp = (ThreadRequestFillScoreBoard*)(itr->second);
                        sStorage->m_scoreBoard.clear();

                        sStorage->m_scoreBoard = tmp->scores;
                        sStorage->m_scoreBoard.SortByKills();
                        break;
                    }
                    case REQUEST_RESPAWN_PLAYER:
                    {
                        ThreadRequestRespawnPlayer* tmp = (ThreadRequestRespawnPlayer*)(itr->second);
                        if (tmp->id == sStorage->m_myId)
                        {
                            sGameplayMgr->PlayerDied(false);
                            sStorage->m_respawnRequest = false;
                            if (ModelDisplayListRecord* rec = sGameplayMgr->GetPlayerRec())
                            {
                                rec->x = tmp->x-0.5f;
                                rec->z = tmp->z-0.5f;
                            }
                        }
                        else
                        {
                            Player* pl = sNetwork->GetPlayerById(tmp->id);
                            if (!pl)
                                break;

                            sAnimator->ChangeModelAnim(pl->rec->AnimTicket, ANIM_IDLE, 0, 0, 0);
                            pl->rec->x = tmp->x-0.5f;
                            pl->rec->z = tmp->z-0.5f;
                        }
                        break;
                    }
                    case REQUEST_CHAT_MESSAGE:
                    {
                        ThreadRequestChatMessage* tmp = (ThreadRequestChatMessage*)(itr->second);
                        std::string mesg = "";
                        if (tmp->type == CHAT_MSG_PLAYER_CHAT)
                        {
                            if (sStorage->m_myId == tmp->sourceId)
                            {
                                mesg += sStorage->m_nickName.c_str();
                                mesg += ": ";
                            }
                            else
                            {
                                Player* pl = sNetwork->GetPlayerById(tmp->sourceId);
                                if (pl)
                                {
                                    mesg += pl->name.c_str();
                                    mesg += ": ";
                                }
                            }
                        }
                        mesg += tmp->message;
                        sGameplayMgr->ConsoleWrite(mesg.c_str());
                        break;
                    }
                    case REQUEST_ADD_BONUS:
                    {
                        ThreadRequestAddBonus* tmp = (ThreadRequestAddBonus*)(itr->second);
                        Map* pMap = sMapManager->GetMap();

                        if (pMap)
                        {
                            pMap->AddDynamicCell(tmp->x, tmp->y, DYNAMIC_TYPE_BONUS, 0, tmp->type);
                            sMapManager->FillDynamicRecords();
                        }
                        break;
                    }
                    case REQUEST_DYNAMIC_DESTROY:
                    {
                        ThreadRequestDynamicFieldDestroy* tmp = (ThreadRequestDynamicFieldDestroy*)(itr->second);

                        Map* pMap = sMapManager->GetMap();

                        if (pMap)
                            pMap->DestroyDynamicRecords(tmp->x, tmp->y, tmp->type);
                        break;
                    }
                }
            }
            itr = sStorage->m_interThreadObjectRequests.erase(itr);
        }
    }

    sLockMgr->UnNeedToken(LOCK_STORAGE, THREAD_MAIN);
}
