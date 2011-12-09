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
#include <Network.h>
#include <Timer.h>

#include <boost/thread.hpp>

#include <io.h>
#include <stdlib.h>
#include <fcntl.h>

HDC hDC=NULL;
HGLRC hRC=NULL;
HWND hWnd=NULL;
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

bool InitGL(GLvoid)
{
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);

    sDisplay->Initialize();

    return true;
}

void Application::Update()
{
    if (sDisplay->IsIn2DMode())
        sDisplay->Setup3DMode();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    uint32 diff = uint32(clock() - m_lastUpdate);

    sDisplay->Update(diff);
    m_currStage->OnDraw(diff);
    m_currStage->OnUpdate(diff);
    sTimer->Update(diff);

    //Sleep(1);

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
        ShowCursor(FALSE);
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
    }

    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (!sApplication->Init())
        return 0;

    return sApplication->Run();
}

Application::Application()
{
    m_currStage = NULL;
    m_lastUpdate = clock();

    for (uint16 i = 0; i < 256; i++)
        keys[i] = false;

    for (uint8 i = 0; i < 2; i++)
        mouse[i] = 0;

    for (uint8 i = 0; i < 2; i++)
        mouseXY[i] = 0;
}

Application::~Application()
{
}

bool Application::Init()
{
    srand((unsigned int)time(NULL));

    // Alokujeme si konzoli
    /*AllocConsole();
    // A presmerujeme do ni stdout (kvuli fprintf do handle stdout)
    int hCrt, i;
    FILE *hf;
    hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    hf   = _fdopen(hCrt, "w");
    *stdout = *hf;
    i = setvbuf(stdout, NULL, _IONBF, 0);*/

    if (!sConfig->Load())
        return false;

    if (!CreateGLWindow("BomberEngine",sConfig->WindowWidth,sConfig->WindowHeight,sConfig->ColorDepth,sConfig->fullscreen,sConfig->RefreshRate))
        return false;

    ShowCursor(true);

    sStorage->Load();

    sNetwork->Initialize();
    if (sNetwork->IsInitialized())
        boost::thread NetworkThread(runNetworkWorker);

    m_currStage = new MenuStage;
    SetStage(STAGE_MENU);

    return true;
}

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

void Application::PMessageBox(const char* caption, const char* format, ...)
{
    va_list argList;
    va_start(argList,format);
    char buf[2048];
    vsnprintf(buf,2048,format,argList);
    va_end(argList);

    MessageBox(NULL, buf, caption, MB_OK);
}

void Application::SetStage(uint32 newstage, uint32 newphase)
{
    if (m_currStage)
        delete m_currStage;

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
        default:
            assert(false && "Invalid stage!");
    }

    assert(m_currStage != NULL);

    sDisplay->FlushModelDisplayList();

    m_currStage->SetSubStage(newphase);
    m_currStage->OnEnter();
}

void Application::KeyStateChange(uint8 key, bool press)
{
    if (keys[key] != press)
    {
        keys[key] = press;

        m_currStage->OnKeyStateChange(key, press);
    }
}

void Application::MouseButtonStateChange(bool left, bool press)
{
    uint8 pos = (left ? 0 : 1);
    if (mouse[pos] != press)
    {
        mouse[pos] = press;

        m_currStage->OnMouseButtonPress(mouseXY[0], mouseXY[1], left);
    }
}

void Application::SetStagePhase(uint32 newphase)
{
    m_currStage->SetSubStage(newphase);
}
