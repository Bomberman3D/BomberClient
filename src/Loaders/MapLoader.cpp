#include <Global.h>
#include <Map.h>
#include <Storage.h>
#include <Effects/Animations.h>

#include <algorithm>

//Nacte mapu ze souboru
bool MapManager::LoadMap(uint32 id)
{
    SetMapData(id);

    FILE* map = fopen(MapFilename.c_str(),"r");

    if(!map)
        return false;

    if (!pMap)
        pMap = new Map;

    assert(pMap != NULL);

    // Nejdriv precteme zahlavi mapy
    // 0..3     - delka jmena
    // 4..x     - jmeno
    // x+1..x+4 - ID skyboxu
    uint32 namesize = 0;
    fread(&namesize,4,1,map);
    char* mapname = new char[namesize+1];
    fread(mapname,1,namesize,map);
    mapname[namesize] = 0;
    pMap->m_mapName = mapname;
    fread(&pMap->m_skybox,2,1,map);

    Chunk* pChunk = new Chunk;

    // Inicializace mapy na rozmer 1x1
    pMap->field.resize(1);
    pMap->field[0].resize(1);

    // A cteme bunky tak dlouho, dokud nam to dovoli
    while(fread(pChunk,sizeof(Chunk),1,map) > 0)
    {
        // V pripade potreby rozsirime po nektere z os
        if (pChunk->x > pMap->field.size()-1)
        {
            pMap->field.resize(pChunk->x+1);
            pMap->field[pMap->field.size()-1].resize(pMap->field[0].size());
        }
        if (pChunk->y > pMap->field[0].size()-1)
        {
            for (uint32 i = 0; i < pMap->field.size(); i++)
                pMap->field[i].resize(pChunk->y+1);
        }
        pMap->field[pChunk->x][pChunk->y].type = pChunk->type;
        pMap->field[pChunk->x][pChunk->y].texture = pChunk->texture;
    }

    // A inicializujeme dynamickou mapu na stejny rozmer
    pMap->dynfield.resize(pMap->field.size());
    for (uint32 it = 0; it < pMap->field.size(); it++)
        pMap->dynfield[it].resize(pMap->field[0].size());

    return true;
}

void MapManager::FillDynamicRecords()
{
    // Zkontrolovat validitu ukazatelu
    if (!pMap)
        return;

    // A hned pak jestli je mapa vetsi nez 0x0, aby to melo vubec cenu
    if (pMap->dynfield.size() < 1 || pMap->dynfield[0].size() < 1)
        return;

    for (uint32 i = 0; i < pMap->dynfield.size(); i++)
    {
        for (uint32 j = 0; j < pMap->dynfield[i].size(); j++)
        {
            if (pMap->dynfield[i][j].size() > 0)
            {
                for (uint32 k = 0; k < pMap->dynfield[i][j].size(); k++)
                {
                    switch (pMap->dynfield[i][j][k].type)
                    {
                        case DYNAMIC_TYPE_BOMB:
                            if (pMap->dynfield[i][j][k].special == NULL)
                            {
                                pMap->dynfield[i][j][k].special = (void*)(sDisplay->DrawModel(2, i-0.5f, 0.0f, j-0.5f, ANIM_IDLE, 0.45f));
                                sAnimator->ChangeModelAnim(((ModelDisplayListRecord*)pMap->dynfield[3][2][0].special)->AnimTicket, ANIM_IDLE, 0, 8);
                            }
                            break;
                        case DYNAMIC_TYPE_BOX:
                            if (pMap->dynfield[i][j][k].special == NULL)
                            {
                                pMap->dynfield[i][j][k].special = (void*)(sDisplay->DrawModel(6, i-0.5f, 0.0f, j-0.5f, ANIM_IDLE, 0.35f));
                            }
                            break;
                    }
                }
            }
        }
    }
}

void MapManager::SetMapFile(const char *filename)
{
    char tmp[2048];
    sprintf(tmp,"%s/%s",DATA_PATH,filename);
    MapFilename = tmp;
}

void MapManager::SetMapId(uint32 id)
{
    if (!pMap)
        pMap = new Map;

    assert(pMap != NULL);

    pMap->mapId = id;
}

void MapManager::SetMapData(uint32 id)
{
    SetMapId(id);
    SetMapFile(sStorage->MapData[id].filename.c_str());
}

//////////////
void Map::AddDynamicCell(uint32 x, uint32 y, uint32 type, uint32 state, uint32 misc, void* special)
{
    if (x >= dynfield.size() || y >= dynfield[x].size())
        return;

    uint32 pos = dynfield[x][y].size(); // potrebujeme o 1 vice nez max. prvek, cili +1-1

    if (pos <= 65535)
    {
        dynfield[x][y].resize(pos+1);
        dynfield[x][y][pos].type    = type;
        dynfield[x][y][pos].state   = state;
        dynfield[x][y][pos].misc    = misc;
        dynfield[x][y][pos].special = special;
        return;
    }
}

Map::DynamicCellSet* Map::GetDynamicCellSet(uint32 x, uint32 y)
{
    if (x >= dynfield.size() || y >= dynfield[x].size())
        return NULL;

    return &dynfield[x][y];
}

void Map::DestroyDynamicRecords(uint32 x, uint32 y, int32 type)
{
    if (dynfield[x][y].empty())
        return;

    for (uint32 i = 0; i < dynfield[x][y].size(); i++)
    {
        // Odstranujeme jen specifikovany typ nebo pri type == -1 vsechny
        if (type != -1 && dynfield[x][y][i].type != type)
            continue;

        if (dynfield[x][y][i].type == DYNAMIC_TYPE_BOX)
        {
            // TODO: animace?
            if (dynfield[x][y][i].special != NULL)
                sDisplay->RemoveRecordFromDisplayList((ModelDisplayListRecord*)dynfield[x][y][i].special);
        }
        else if (dynfield[x][y][i].type == DYNAMIC_TYPE_BOMB)
        {
            // TODO: animace! disappear, implementovat v animacich, aby po skonceni disappear sekvence objekt fakt zmizel
            //       pak odebrat tohle, nahradit to zmenou animace. Nezapomenout na vetsi frame skip pri normalni animaci
            if (dynfield[x][y][i].special != NULL)
                sDisplay->RemoveRecordFromDisplayList((ModelDisplayListRecord*)dynfield[x][y][i].special);
        }
    }
}
