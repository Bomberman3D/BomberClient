#include <Global.h>
#include <GameTypes.h>
#include <Gameplay.h>
#include <Map.h>

void ClassicSingleGameType::OnGameInit()
{
    const Map* pMap = sMapManager->GetMap();
    if (!pMap)
        return;

    pMap->DestroyAllDynamicRecords();
}

void ClassicSingleGameType::OnUpdate()
{
}
