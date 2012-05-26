#ifndef BOMB_STAGES_H
#define BOMB_STAGES_H

#include <Display.h>
#include <GameTypes.h>
#include <LoadingThread.h>

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

/** \class StageControl
 *  \brief Predloha pro vsechny faze hry
 */
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

/** \fn StageControl::OnEnter
 *  \brief Funkce volana pri vstupu do herni faze
 */

/** \fn StageControl::OnLeave
 *  \brief Funkce volana pri odchodu z herni faze
 */

/** \fn StageControl::OnDraw(uint32 diff)
 *  \brief Funkce volana po vykresleni vseho tridou DisplayMgr
 */

/** \fn StageControl::OnBeforeDraw(uint32 diff)
 *  \brief Funkce volana pred vykreslenim vseho jineho
 */

/** \fn StageControl::OnKeyStateChange(uint16 key, bool press)
 *  \brief Funkce volana pri zmene stavu stisknuti klavesy \a key
 */

/** \fn StageControl::OnMouseButtonPress(uint32 x, uint32 y, bool left = true)
 *  \brief Funkce volana pri stisku tlacitka mysi (leve / prave)
 */

/** \fn StageControl::OnUpdate(uint32 diff)
 *  \brief Funkce volana kazdy pruchod hlavniho cyklu aplikace
 */

/** \fn StageControl::GetType
 *  \brief Vrati fazi hry, ve ktere jsme
 */

/** \fn StageControl::GetSubStage
 *  \brief Vraci argument faze hry
 */

/** \fn StageControl::SetSubStage(uint32 subStage)
 *  \brief Nastavi argument faze hry
 */

/** \class MenuStage
 *  \brief Faze hry - menu
 */
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

/** \class IntroOutroStage
 *  \brief Faze hry - intro / outro
 */
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
        clock_t startTime;
};

/** \class LoadingStage
 *  \brief Faze hry - nacitani
 */
class LoadingStage: public StageControl
{
    public:
        LoadingStage()
        {
            m_type = STAGE_LOADING;
        }
        void OnEnter();
        void OnDraw(uint32 diff);

        void PreLoad(LoadType type, uint32 sourceId);
        bool IsAllLoaded();
        float GetLoadingPercentage();

    protected:
        uint32 ImgAnimTicket;
        std::vector<LoadPair> m_toLoad;
};

/** \fn LoadingStage::PreLoad(LoadType type, uint32 sourceId)
 *  \brief Postara se o nacteni daneho prvku pred prechodem do dalsi faze
 */

/** \fn LoadingStage::IsAllLoaded
 *  \brief Overi, zdali bylo nacteno vse, co jsme zazadali u funkce PreLoad
 */

/** \fn LoadingStage::GetLoadingPercentage
 *  \brief Vraci zpatky procentuelni vyjadreni postupu v nacitani vseho, o co jsme zazadali funkci PreLoad
 */

/** \class GameSettingsStage
 *  \brief Faze hry - herni nastaveni
 */
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

/** \class OptionsStage
 *  \brief Faze hry - nastaveni aplikace
 */
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

        uint32 m_selResolution;
        uint32 m_selColorDepth;
        uint32 m_selRefreshRate;
        bool m_fullscreen;
};

/** \class GameStage
 *  \brief Faze hry - hlavni herni faze
 */
class GameStage: public StageControl
{
    public:
        GameStage()
        {
            m_type = STAGE_GAME;
        }
        void OnEnter();
        void OnLeave();
        void OnDraw(uint32 diff);
        void OnBeforeDraw(uint32 diff);
        void OnKeyStateChange(uint16 key, bool press);
        void OnMouseButtonPress(uint32 x, uint32 y, bool left = true);
        void OnUpdate(uint32 diff);

    protected:
};

#endif
