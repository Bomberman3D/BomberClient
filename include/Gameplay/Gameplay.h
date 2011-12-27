#ifndef BOMB_GAMEPLAY_H
#define BOMB_GAMEPLAY_H

#include <Global.h>
#include <Singleton.h>
#include <GameTypes.h>
#include <AI.h>

struct BombRecord
{
    uint32 x;
    uint32 y;
    uint32 state;
    //pozdeji i data o vlastnikovi?
};

class EnemyTemplate
{
    public:
        EnemyTemplate() { pRecord = NULL; m_nextUpdate = 0; };
        void Init(uint32 modelId, uint32 x, uint32 y);

        clock_t m_nextUpdate;

        ModelDisplayListRecord* pRecord;

        // Pathfinding veci - vektor "bodu", aktualni bod
        Path m_path;
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
        void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true) { if (m_game) m_game->OnBoxDestroy(x,y,by_bomb); };
        void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY);

        bool AddBomb(uint32 x, uint32 y);

        float GetPlayerSpeedCoef() { return m_plSpeedCoef; };
        uint32 GetFlameReach() { return m_plFlameReach; };
        uint32 GetMaxBombs() { return m_plMaxBombs; };

        void SetPlayerSpeedCoef(float value, bool relative = false) { m_plSpeedCoef = (relative)?(m_plSpeedCoef+value):(value); };
        void SetFlameReach(uint32 value, bool relative = false) { m_plFlameReach = (relative)?(m_plFlameReach+value):(value); };
        void SetMaxBombs(float value, bool relative = false) { m_plMaxBombs = (relative)?(m_plMaxBombs+value):(value); };

    private:
        GameTypeTemplate* m_game;
        std::list<BombRecord*> BombMap;

        float  m_plSpeedCoef;
        uint32 m_plFlameReach;
        uint32 m_plMaxBombs;

        uint32 m_plActiveBombs;
};

#define sGameplayMgr Singleton<GameplayMgr>::instance()

#endif
