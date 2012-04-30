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

typedef std::list<EnemyTemplate*> EnemyList;

class GameTypeTemplate
{
    public:
        GameTypeTemplate()
        {
        }
        virtual ModelDisplayListRecord* SpawnNewPlayer() { return NULL; };
        virtual void OnGameInit(ModelDisplayListRecord* pPlayerRec) {};
        virtual void OnGameLeave() {};
        virtual void OnUpdate() {};
        virtual void OnBombBoom(BombRecord* bomb) {};
        virtual void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true) {};
        virtual void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY) {};
        virtual void OnDangerousFieldActivate(uint32 x, uint32 y) {};

        virtual EnemyList* GetEnemies() { return NULL; };

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

/** \fn GameTypeTemplate::GameTypeTemplate
 *  \brief Konstruktor
 *
 * Prazdny
 */

/** \fn GameTypeTemplate::SpawnNewPlayer
 *  \brief Postara se o pridani zaznamu do display listu pro noveho hrace
 */

/** \fn GameTypeTemplate::OnGameInit(ModelDisplayListRecord* pPlayerRec)
 *  \brief Funkce volana pri inicializaci, zde se generuji pozice beden a tak podobne
 */

/** \fn GameTypeTemplate::OnGameLeave
 *  \brief Funkce volana pri odchodu ze hry, casto jen cleanup
 */

/** \fn GameTypeTemplate::OnUpdate
 *  \brief Funkce volana kazdy pruchod hernim cyklem z GameplayMgr::Update
 */

/** \fn GameTypeTemplate::OnBombBoom(BombRecord* bomb)
 *  \brief Funkce volana pri vybuchu bomby
 */

/** \fn GameTypeTemplate::OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true)
 *  \brief Funkce volana pri zniceni bedny - zde se napriklad generuje vytvoreni bonusu na danych souradnicich
 */

/** \fn GameTypeTemplate::OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY)
 *  \brief Funkce volana pri prechodu hrace z jineho pole 2D mapy na nove
 */

/** \fn GameTypeTemplate::OnDangerousFieldActivate(uint32 x, uint32 y)
 *  \brief Funkce volana pri aktivaci nebezpecneho pole
 */

/** \fn GameTypeTemplate::GetEnemies
 *  \brief Vraci ukazatel na list nepratel na mape
 */

/** \fn GameTypeTemplate::GetType
 *  \brief Vraci typ hry
 */

class ClassicSingleGameType: public GameTypeTemplate
{
    public:
        ClassicSingleGameType()
        {
            m_type = GAME_TYPE_SP_CLASSIC;
        }
        ModelDisplayListRecord* SpawnNewPlayer();
        void OnGameInit(ModelDisplayListRecord* pPlayerRec);
        void OnGameLeave();
        void OnUpdate();
        void OnBombBoom(BombRecord* bomb);
        void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true);
        void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY);
        void OnDangerousFieldActivate(uint32 x, uint32 y);

        virtual EnemyList* GetEnemies() { return &m_enemies; };

    private:
        EnemyList m_enemies;

        uint32 m_playerX;
        uint32 m_playerY;
};

class MemeSingleGameType: public GameTypeTemplate
{
    public:
        MemeSingleGameType()
        {
            m_type = GAME_TYPE_SP_MEME;
        }
        ModelDisplayListRecord* SpawnNewPlayer();
        void OnGameInit(ModelDisplayListRecord* pPlayerRec);
        void OnGameLeave();
        void OnUpdate();
        void OnBombBoom(BombRecord* bomb);
        void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true);
        void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY);
        void OnDangerousFieldActivate(uint32 x, uint32 y);

        virtual EnemyList* GetEnemies() { return &m_enemies; };

    private:
        EnemyList m_enemies;

        uint32 m_playerX;
        uint32 m_playerY;
};

class ClassicMultiGameType: public GameTypeTemplate
{
    public:
        ClassicMultiGameType()
        {
            m_type = GAME_TYPE_MP_CLASSIC;
        }
        void OnGameInit(ModelDisplayListRecord* pPlayerRec);
        void OnGameLeave();
        void OnUpdate();
};

#endif
