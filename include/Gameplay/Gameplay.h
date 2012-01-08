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

enum PlayerMoveElements
{
    MOVE_FORWARD,
    MOVE_RIGHT,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_MAX
};

struct BombRecord
{
    uint32 x;
    uint32 y;
    uint32 state;
    uint32 reach;
    //pozdeji i data o vlastnikovi?
};

// Struktura pro "nebezpecne pole" :-)
// jde v podstate jen o cas aktivace a deaktivace pro vymazani
struct DangerousField
{
    clock_t activeSince; // aktivni od [ms] - kvuli postupnemu dosahu plamene
    uint32  activeTime;  // [ms]
    bool    registered;  // byly uz provedeny veci "po aktivaci" ?
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

        uint32 GetSetting(SettingsEnum pos) { if (pos >= SETTING_MAX) return 0; else return m_settings[pos]; };
        void SetSetting(SettingsEnum pos, uint32 val) { if (pos < SETTING_MAX) m_settings[pos] = val; };
        std::vector<uint32>* SettingsPointer() { return &m_settings; };

        bool IsDangerousField(uint32 x, uint32 y);
        void SetDangerous(uint32 x, uint32 y, clock_t since, uint32 howLong);

        void PlayerDied(uint32 x, uint32 y);
        void SetPlayerMoveAngle(float angle) { m_moveAngle = angle; };
        void ChangePlayerMoveAngle(int32 coordDif);
        void UpdatePlayerMotion(uint32 diff);
        void SetMoveElement(uint8 direction) { if (direction < MOVE_MAX) m_moveElements[direction] = true; };
        void UnsetMoveElement(uint8 direction) { if (direction < MOVE_MAX) m_moveElements[direction] = false; };

        ModelDisplayListRecord* GetPlayerRec() { return m_playerRec; };

    private:
        std::vector<uint32> m_settings;
        GameTypeTemplate* m_game;
        std::list<BombRecord*> BombMap;
        std::map<std::pair<uint32, uint32>, DangerousField*> DangerousMap;
        std::vector<bool> m_moveElements;
        float m_moveAngle;

        ModelDisplayListRecord* m_playerRec;
        uint32 m_playerX, m_playerY;

        float  m_plSpeedCoef;
        uint32 m_plFlameReach;
        uint32 m_plMaxBombs;

        uint32 m_plActiveBombs;
};

#define sGameplayMgr Singleton<GameplayMgr>::instance()

#endif
