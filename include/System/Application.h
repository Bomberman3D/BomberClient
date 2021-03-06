/**********************************
*        Bomberman3D Engine       *
*  Created by: Cmaranec (Kennny)  *
**********************************/
// Application singleton defines

#ifndef BOMB_APPLICATION_H
#define BOMB_APPLICATION_H

#include <Global.h>
#include <Stages.h>
#include <Singleton.h>

#include <ctime>

enum RenderingContext
{
    RC_MAIN,
    RC_LOADING,
    RC_MAX
};

/*! \class Application
 * \brief Trida kostry aplikace
 *
 * Obstarava zakladni veci, jako je napriklad management hernich fazi, stisknuti tlacitek a tak podobne
 */
class Application
{
    public:
        Application();
        ~Application();

        bool Init();
        int  Run();

        void PMessageBox(const char* caption, const char* format, ... );

        void Update();
        void ProcessInterThreadRequests();

        bool IsAppActive() { return appActive; };
        void SetAppActive(bool active) { appActive = active; };

        void SetStage(uint32 newstage, uint32 newphase = 0, bool cleanup = true);
        uint32 GetStage() { return m_currStage->GetType(); };
        void SetStagePhase(uint32 newphase);
        uint32 GetStagePhase() { return m_currStage->GetSubStage(); };

        void KeyStateChange(uint8 key, bool press);
        void PrintableCharPress(uint16 chr);
        void MouseButtonStateChange(bool left, bool press);
        void SetMouseXY(uint32 x, uint32 y)
        {
            mouseXY[0] = x;
            mouseXY[1] = y;
        }
        uint32 GetMouseX() { return mouseXY[0]; };
        uint32 GetMouseY() { return mouseXY[1]; };

        bool IsKeyPressed(uint8 key) { return keys[key]; };

        void ApplyRenderContext(RenderingContext cont);

    protected:
        uint32 Stage;
        bool StageChanged;
        StageControl* m_currStage;
        bool appActive;

        bool keys[256];
        bool mouse[2];
        bool console;
        uint32 mouseXY[2];

        clock_t m_lastUpdate;
};

#define sApplication Singleton<Application>::instance()

/** \fn Application::GetStage
 *  \brief Ziskani aktualni stage aplikace
 */

/** \fn Application::GetStagePhase
 *  \brief Ziskani argumentu pro aktualni stage aplikace
 */

/** \fn Application::SetMouseXY
 *  \brief Nastaveni pozice mysi pouze v ramci aplikace
 */

/** \fn Application::GetMouseX
 *  \brief Ziskani Xove pozice mysoveho kurzoru
 */

/** \fn Application::GetMouseY
 *  \brief Ziskani Yove pozice mysoveho kurzoru
 */

/** \fn Application::IsKeyPressed
 *  \brief Zjisteni, zdali je dana klavesa stisknuta
 */

#endif
