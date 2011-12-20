#ifndef BOMB_GAMEPLAY_H
#define BOMB_GAMEPLAY_H

#include <Global.h>
#include <Singleton.h>
#include <GameTypes.h>

struct BombRecord
{
    uint32 x;
    uint32 y;
    uint32 state;
    //pozdeji i data o vlastnikovi?
};

struct PathNode
{
    float x, y, z;
    uint8 flags; // jeste nepouzito - napr. polozit bombu, prepocitat cestu, atd...
};

class EnemyTemplate
{
    public:
        EnemyTemplate() { pRecord = NULL; m_nextUpdate = 0; };
        void Init(uint32 modelId, uint32 x, uint32 y);

        clock_t m_nextUpdate;

        ModelDisplayListRecord* pRecord;

        // Pathfinding veci - vektor "bodu", aktualni bod
        std::vector<PathNode> m_path;
        uint32 m_actualNodePos;
};

class GameplayMgr
{
    public:
        GameplayMgr();
        ~GameplayMgr();

        void Update();
        void SetGameType(GameType type);
        GameType GetGameType();

        void OnGameInit();

        void AddBomb(uint32 x, uint32 y);

    private:
        GameTypeTemplate* m_game;
        std::list<BombRecord*> BombMap;
};

#define sGameplayMgr Singleton<GameplayMgr>::instance()

#endif
