#ifndef BOMB_STAGES_H
#define BOMB_STAGES_H

#include <Display.h>
#include <GameTypes.h>

enum Stages
{
    STAGE_INTRO,
    STAGE_MENU,
    STAGE_OPTIONS,
    STAGE_GAMESETTINGS,
    STAGE_LOADING,
    STAGE_GAME,
    STAGE_MAX
};

// Staging
class StageControl
{
    public:
        virtual void OnEnter() {};
        virtual void OnLeave() {};
        virtual void OnDraw(uint32 diff) {};
        virtual void OnBeforeDraw(uint32 diff) {};
        virtual void OnKeyStateChange(uint16 key, bool press) {};
        virtual void OnMouseButtonPress(uint32 x, uint32 y, bool left = true) {};
        virtual void OnUpdate(uint32 diff) {};

        Stages GetType() { return m_type; }
        // Stage je nastavena v konstruktoru potomka, nic externiho ji nesmi zmenit
        //void SetType(Stages stage) { m_type = stage; }

        uint32 GetSubStage() { return m_subStage; }
        void SetSubStage(uint32 subStage) { m_subStage = subStage; }

    protected:
        Stages m_type;
        uint32 m_subStage;
};

class MenuStage: public StageControl
{
    public:
        MenuStage()
        {
            m_type = STAGE_MENU;
        }
        void OnEnter();
        void OnLeave();
        void OnDraw(uint32 diff);
        void OnKeyStateChange(uint16 key, bool press);
        void OnMouseButtonPress(uint32 x, uint32 y, bool left = true);

    protected:
};

class IntroOutroStage: public StageControl
{
    public:
        IntroOutroStage()
        {
            m_type = STAGE_INTRO;
        }
        void OnEnter();
        void OnDraw(uint32 diff);
        void OnKeyStateChange(uint16 key, bool press);
        void OnMouseButtonPress(uint32 x, uint32 y, bool left = true);

    protected:
};

class LoadingStage: public StageControl
{
    public:
        LoadingStage()
        {
            m_type = STAGE_LOADING;
        }
        void OnEnter();
        void OnDraw(uint32 diff);

    protected:
        uint32 ImgAnimTicket;
};

class GameSettingsStage: public StageControl
{
    public:
        GameSettingsStage()
        {
            m_type = STAGE_GAMESETTINGS;
        }
        void OnEnter();
        void OnLeave();
        void OnDraw(uint32 diff);
        void OnKeyStateChange(uint16 key, bool press);
        void OnMouseButtonPress(uint32 x, uint32 y, bool left = true);

    protected:
        // Multiplayer
        uint32 m_selected;
        std::string m_nickfield;
        bool m_nickselected;

        // Singleplayer
        uint32 m_selectedMapId;
        GameType m_selectedMode;
};

class OptionsStage: public StageControl
{
    public:
        OptionsStage()
        {
            m_type = STAGE_OPTIONS;
        }
        void OnEnter();
        void OnDraw(uint32 diff);
        void OnKeyStateChange(uint16 key, bool press);
        void OnMouseButtonPress(uint32 x, uint32 y, bool left = true);

    protected:
};

class GameStage: public StageControl
{
    public:
        GameStage()
        {
            m_type = STAGE_GAME;
            plX = 0;
            plY = 0;
        }
        void OnEnter();
        void OnLeave();
        void OnDraw(uint32 diff);
        void OnBeforeDraw(uint32 diff);
        void OnKeyStateChange(uint16 key, bool press);
        void OnMouseButtonPress(uint32 x, uint32 y, bool left = true);
        void OnUpdate(uint32 diff);

    protected:
        ModelDisplayListRecord* pPlayerRec;

        uint32 plX, plY;
};

#endif
