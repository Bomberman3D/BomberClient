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

struct GameTypeResources
{
    std::vector<uint32> PlayerModelIDs;
    std::vector<uint32> EnemyIDs;
    std::vector<uint32> EnemyModelIDs; // plneno az za behu!

    std::vector<uint32> MiscModelIDs;
    std::vector<uint32> MiscTextureIDs;

    std::vector<uint32> MusicIDs;
    std::vector<uint32> RandomSayIDs;
};

/** \class GameTypeTemplate
 *  \brief Predloha pro vsechny herni typy, obsahuje jen prazdne prototypy virtualnich funkci
 */
class GameTypeTemplate
{
    public:
        GameTypeTemplate()
        {
            gameResources.PlayerModelIDs.clear();
            gameResources.EnemyIDs.clear();
            gameResources.EnemyModelIDs.clear();
            gameResources.MiscModelIDs.clear();
            gameResources.MiscTextureIDs.clear();
            gameResources.MusicIDs.clear();
            gameResources.RandomSayIDs.clear();
        }
        virtual ModelDisplayListRecord* SpawnNewPlayer() { return NULL; };
        virtual void FillGameTypeResources() {};
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

        GameTypeResources gameResources;
        void AddEnemyModelID(uint32 modelId)
        {
            for (std::vector<uint32>::const_iterator itr = gameResources.EnemyModelIDs.begin(); itr != gameResources.EnemyModelIDs.end(); ++itr)
                if ((*itr) == modelId)
                    return;

            gameResources.EnemyModelIDs.push_back(modelId);
        }

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

/** \fn GameTypeTemplate::FillGameTypeResources
 *  \brief Naplni strukturu gameResources zdroji, ktere je treba nacist pro dany typ hry
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

/** \var GameTypeTemplate::gameResources
 *  \brief Uloziste ID zdroju (modely, textury), ktere jsou treba nacist v nacitaci fazi pred vstupem do hry
 */

/** \fn GameTypeTemplate::AddEnemyModelID
 *  \brief Prida ID modelu do mapy k nacteni
 */

/** \class ClassicSingleGameType
 *  \brief Trida klasickeho herniho typu
 */
class ClassicSingleGameType: public GameTypeTemplate
{
    public:
        ClassicSingleGameType(): GameTypeTemplate()
        {
            m_type = GAME_TYPE_SP_CLASSIC;
        }
        ModelDisplayListRecord* SpawnNewPlayer();
        void FillGameTypeResources();
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

/** \class MemeSingleGameType
 *  \brief Trida herniho typu "Meme wars"
 */
class MemeSingleGameType: public GameTypeTemplate
{
    public:
        MemeSingleGameType(): GameTypeTemplate()
        {
            m_type = GAME_TYPE_SP_MEME;
        }
        ModelDisplayListRecord* SpawnNewPlayer();
        void FillGameTypeResources();
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

/** \class ClassicMultiGameType
 *  \brief Trida klasickeho herniho typu hrana vice hraci pres sit
 */
class ClassicMultiGameType: public GameTypeTemplate
{
    public:
        ClassicMultiGameType(): GameTypeTemplate()
        {
            m_type = GAME_TYPE_MP_CLASSIC;
        }
        ModelDisplayListRecord* SpawnNewPlayer();
        void FillGameTypeResources();
        void OnGameInit(ModelDisplayListRecord* pPlayerRec);
        void OnGameLeave();
        void OnUpdate();
        void OnBombBoom(BombRecord* bomb);
        void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true);
        void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY);
};

#endif
