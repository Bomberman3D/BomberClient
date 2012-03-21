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

// Samotna kostra aplikace
class Application
{
    public:
        Application();
        ~Application();

        bool Init();
        int  Run();

        void PMessageBox(const char* caption, const char* format, ... );

        void Update();

        void SetStage(uint32 newstage, uint32 newphase = 0);
        uint32 GetStage() { return m_currStage->GetType(); };
        void SetStagePhase(uint32 newphase);
        uint32 GetStagePhase() { return m_currStage->GetSubStage(); };

        void KeyStateChange(uint8 key, bool press);
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

        bool keys[256];
        bool mouse[2];
        uint32 mouseXY[2];

        uint32 m_diff;
        clock_t m_lastUpdate;
};

#define sApplication Singleton<Application>::instance()

#endif
