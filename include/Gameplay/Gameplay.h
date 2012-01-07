#ifndef BOMB_GAMEPLAY_H
#define BOMB_GAMEPLAY_H

#include <Global.h>
#include <Singleton.h>
#include <GameTypes.h>
#include <AI.h>

enum SettingsEnum
{
    SETTING_ENEMY_COUNT = 0,
    SETTING_ENEMY_AI_LEVEL = 1,
    SETTING_ENEMY_SPEED = 2,
    SETTING_BOX_DENSITY = 3,
    SETTING_MAP_ID = 4,
    SETTING_MAX
};

struct BombRecord
{
    uint32 x;
    uint32 y;
    uint32 state;
    //pozdeji i data o vlastnikovi?
};

class GameplayMgr
{
    public:
        GameplayMgr();
        ~GameplayMgr();

        void Update();
        void SetGameType(GameType type);
        GameType GetGameType();

        void OnGameInit(ModelDisplayListRecord* pPlayerRec);
        void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true) { if (m_game) m_game->OnBoxDestroy(x,y,by_bomb); };
        void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY);

        bool AddBomb(uint32 x, uint32 y);

        float GetPlayerSpeedCoef() { return m_plSpeedCoef; };
        uint32 GetFlameReach() { return m_plFlameReach; };
        uint32 GetMaxBombs() { return m_plMaxBombs; };

        void SetPlayerSpeedCoef(float value, bool relative = false) { m_plSpeedCoef = (relative)?(m_plSpeedCoef+value):(value); };
        void SetFlameReach(uint32 value, bool relative = false) { m_plFlameReach = (relative)?(m_plFlameReach+value):(value); };
        void SetMaxBombs(float value, bool relative = false) { m_plMaxBombs = (relative)?(m_plMaxBombs+value):(value); };

        uint32 GetSetting(SettingsEnum pos) { if (pos >= SETTING_MAX) return 0; else return m_settings[pos]; };
        void SetSetting(SettingsEnum pos, uint32 val) { if (pos < SETTING_MAX) m_settings[pos] = val; };
        std::vector<uint32>* SettingsPointer() { return &m_settings; };

    private:
        std::vector<uint32> m_settings;
        GameTypeTemplate* m_game;
        std::list<BombRecord*> BombMap;

        float  m_plSpeedCoef;
        uint32 m_plFlameReach;
        uint32 m_plMaxBombs;

        uint32 m_plActiveBombs;
};

#define sGameplayMgr Singleton<GameplayMgr>::instance()

#endif
