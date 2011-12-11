#ifndef BOMB_GAME_TYPES_H
#define BOMB_GAME_TYPES_H

#include <Global.h>

// Enumerator vsech moznych typu hry
enum GameType
{
    GAME_TYPE_NONE = 0,
    GAME_TYPE_SP_CLASSIC,
    GAME_TYPE_MP_CLASSIC,
    GAME_TYPE_MAX
};

class GameTypeTemplate
{
    public:
        GameTypeTemplate()
        {
        }
        virtual void OnGameInit() {};
        virtual void OnUpdate() {};

        // Multiplayer funkce, prepsany jen v multiplayerovych potomkach
        //virtual void OnPlayerJoin( ... ) {};
        //virtual void OnPlayerDeath( ... ) {};
        //virtual void OnChat( ... ) {};
        //virtual void OnNotify( ... ) {};
        //...

        GameType GetType() { return m_type; };

    protected:
        GameType m_type;
};

class ClassicSingleGameType: public GameTypeTemplate
{
    public:
        ClassicSingleGameType()
        {
            m_type = GAME_TYPE_SP_CLASSIC;
        }
        void OnGameInit();
        void OnUpdate();
};

class ClassicMultiGameType: public GameTypeTemplate
{
    public:
        ClassicMultiGameType()
        {
            m_type = GAME_TYPE_MP_CLASSIC;
        }
        void OnGameInit();
        void OnUpdate();
};

#endif
