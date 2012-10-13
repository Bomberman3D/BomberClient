#include <Global.h>
#include <GameTypes.h>
#include <Gameplay.h>

ModelDisplayListRecord* ClassicMultiGameType::SpawnNewPlayer()
{
    return sDisplay->DrawModel(9, 0.5f, 0, 0.5f, ANIM_IDLE, 3.5f, 90.0f, true, false, 0, 0, ANIM_RESTRICTION_NOT_PAUSED);
}

void ClassicMultiGameType::FillGameTypeResources()
{
    gameResources.PlayerModelIDs.push_back(9);

    // 1-8 jsou songy zakladni sady
    for (uint32 i = 1; i <= 8; i++)
        gameResources.MusicIDs.push_back(i);

    gameResources.RandomSayIDs.push_back(4);
    gameResources.RandomSayIDs.push_back(8);
    gameResources.RandomSayIDs.push_back(11);
    gameResources.RandomSayIDs.push_back(16);
    gameResources.RandomSayIDs.push_back(23);
    gameResources.RandomSayIDs.push_back(24);
}

void ClassicMultiGameType::OnGameInit(ModelDisplayListRecord* pPlayerRec)
{
}

void ClassicMultiGameType::OnGameLeave()
{
}

void ClassicMultiGameType::OnUpdate()
{
}
