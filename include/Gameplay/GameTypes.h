#ifndef BOMB_GAME_TYPES_H
#define BOMB_GAME_TYPES_H

#include <Global.h>

// Enumerator vsech moznych typu hry
enum GameType
{
    GAME_TYPE_NONE = 0,
    GAME_TYPE_SP_MIN = 1, // zachytny bod pro vyber typu hry
    GAME_TYPE_SP_CLASSIC  = 1,
    GAME_TYPE_SP_MEME     = 2, // NYI
    GAME_TYPE_SP_MAX = 2, // zachytny bod pro vyber typu hry
    GAME_TYPE_MP_MIN = 3, // zachytny bod pro vyber typu hry
    GAME_TYPE_MP_CLASSIC  = 3, // NYI
    GAME_TYPE_MP_MEME     = 4, // NYI
    GAME_TYPE_MP_MAX = 4, // zachytny bod pro vyber typu hry
    GAME_TYPE_MAX
};

enum BonusType
{
    BONUS_FLAME = 0,
    BONUS_SPEED = 1,
    BONUS_BOMB  = 2,
    BONUS_MAX
};

struct BombRecord;
class EnemyTemplate;

class GameTypeTemplate
{
    public:
        GameTypeTemplate()
        {
        }
        virtual void OnGameInit() {};
        virtual void OnUpdate() {};
        virtual void OnBombBoom(BombRecord* bomb) {};
        virtual void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true) {};
        virtual void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY) {};

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
        void OnBombBoom(BombRecord* bomb);
        void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true);
        void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY);

    private:
        std::list<EnemyTemplate*> m_enemies;
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
