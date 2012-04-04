#include <Global.h>
#include <Map.h>
#include <Storage.h>
#include <Effects/Animations.h>
#include <Gameplay.h>

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
        pMap->field[pChunk->x][pChunk->y].pRec = NULL;
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
        for (uint32 j = 0; j < pMap->dynfield[0].size(); j++)
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
                                pMap->dynfield[i][j][k].special = (void*)(sDisplay->DrawModel(8, i-0.5f, 0.0f, j-0.5f, ANIM_IDLE, 0.175f, 0.0f, true, true, 0, 8, ANIM_RESTRICTION_NOT_PAUSED));
                                ((ModelDisplayListRecord*)pMap->dynfield[i][j][k].special)->AddFeature(MF_TYPE_EMITTER, 17.4f, 18.539f, 21.02f, sParticleEmitterMgr->AddEmitter(
                                    BillboardDisplayListRecord::Create(55, 0,0,0, 0.2f, 0.2f, true, true), 0,0,0,0.15f, 0.15f, 0.0f, 0.0f, 360.0f, 360.0f,    40, 5,   1.0f, 0.2f,   4,1,   0,0,100000));
                            }
                            break;
                        case DYNAMIC_TYPE_BOX:
                            if (pMap->dynfield[i][j][k].special == NULL)
                            {
                                pMap->dynfield[i][j][k].special = (void*)(sDisplay->DrawModel(7, i-0.5f, 0.0f, j-0.5f, ANIM_IDLE, 6.0f, 0.0f, true, false, 0, 0, ANIM_RESTRICTION_NOT_PAUSED));
                            }
                            break;
                        case DYNAMIC_TYPE_BONUS:
                            if (pMap->dynfield[i][j][k].special == NULL)
                            {
                                uint32 texture = 33;
                                switch (pMap->dynfield[i][j][k].misc)
                                {
                                    case 0:   texture = 33;   break;
                                    case 1:   texture = 34;   break;
                                    case 2:   texture = 35;   break;
                                    default:  texture = 33;   break;
                                }
                                pMap->dynfield[i][j][k].special = (void*)(sDisplay->DrawBillboard(texture, i-0.5f, 0.0f, j-0.5f, 0, 0, 0.8f, 0.8f, false, true, false, ANIM_RESTRICTION_NOT_PAUSED));
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
    if (x >= dynfield.size() || y >= dynfield[x].size() || dynfield[x][y].empty())
        return;

    uint32 destroyedCount = 0;

    for (Map::DynamicCellSet::iterator itr = dynfield[x][y].begin(); itr != dynfield[x][y].end();)
    {
        // Odstranujeme jen specifikovany typ nebo pri type == -1 vsechny
        if (type != -1 && (*itr).type != type)
        {
            ++itr;
            continue;
        }

        if ((*itr).type == DYNAMIC_TYPE_BOX)
        {
            // TODO: animace?
            if ((*itr).special != NULL)
                sDisplay->RemoveRecordFromDisplayList((ModelDisplayListRecord*)(*itr).special);
        }
        else if ((*itr).type == DYNAMIC_TYPE_BOMB)
        {
            // TODO: animace! disappear, implementovat v animacich, aby po skonceni disappear sekvence objekt fakt zmizel
            //       pak odebrat tohle, nahradit to zmenou animace. Nezapomenout na vetsi frame skip pri normalni animaci
            if ((*itr).special != NULL)
                sDisplay->RemoveRecordFromDisplayList((ModelDisplayListRecord*)(*itr).special);
        }
        else if ((*itr).type == DYNAMIC_TYPE_BONUS)
        {
            if ((*itr).special != NULL)
                sDisplay->RemoveRecordFromDisplayList((BillboardDisplayListRecord*)(*itr).special);
        }

        destroyedCount++;
        itr = dynfield[x][y].erase(itr);
    }

    if (type == DYNAMIC_TYPE_BOX)
        for (uint32 i = 0; i < destroyedCount; ++i)
            sGameplayMgr->OnBoxDestroy(x, y, true);
}

bool Map::IsDynamicRecordPresent(uint32 x, uint32 y, int32 type)
{
    if (x > dynfield.size()-1 || y > dynfield[0].size()-1)
        return false;

    if (type == -1 && !dynfield[x][y].empty())
        return true;

    for (Map::DynamicCellSet::iterator itr = dynfield[x][y].begin(); itr != dynfield[x][y].end(); ++itr)
    {
        if ((*itr).type == type)
            return true;
    }

    return false;
}

uint32 Map::GetStaticRecord(uint32 x, uint32 y)
{
    if (x > field.size()-1 || y > field[0].size()-1)
        return 0;

    return field[x][y].type;
}

void Map::DestroyAllDynamicRecords()
{
    if (dynfield.size() == 0)
        return;

    if (dynfield[0].size() == 0)
        return;

    for (uint32 i = 0; i < dynfield.size(); i++)
    {
        for (uint32 j = 0; j < dynfield[0].size(); j++)
        {
            for (uint32 k = 0; k < dynfield[i][j].size(); k++)
            {
                // specialni veci, delete model/billboard zaznamu resi samotny flush
            }
            dynfield[i][j].clear();
        }
    }
}

bool Map::NearStartPos(uint32 x, uint32 y)
{
    if (field.size() < 1 || field[0].size() < 1)
        return false;

    if (field[x][y].type == TYPE_STARTLOC)
        return true;

    if (x > 0 && y > 0 && x < field.size()-1 && y < field[0].size()-1)
    {
        for (int32 i = -1; i <= 1; i++)
            for (int32 j = -1; j <= 1; j++)
                if (field[x+i][y+j].type == TYPE_STARTLOC)
                    return true;
    }

    return false;
}
