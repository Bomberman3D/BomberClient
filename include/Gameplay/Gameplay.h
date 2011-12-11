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
