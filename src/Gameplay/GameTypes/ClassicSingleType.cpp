#include <Global.h>
#include <GameTypes.h>
#include <Gameplay.h>
#include <Map.h>

void ClassicSingleGameType::OnGameInit()
{
    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    pMap->DestroyAllDynamicRecords();

    for (uint32 i = 0; i < pMap->field.size(); i++)
    {
        for (uint32 j = 0; j < pMap->field[i].size(); j++)
        {
            // Muzeme dat bednu jen na misto kde nebude stat hrac, ani pevny objekt mapy
            if ((i > 2 || j > 2) && pMap->field[i][j].type == TYPE_GROUND)
            {
                if (rand()%2)
                {
                    pMap->AddDynamicCell(i,j,DYNAMIC_TYPE_BOX);
                }
            }
        }
    }

    sMapManager->FillDynamicRecords();
}

void ClassicSingleGameType::OnUpdate()
{
}
