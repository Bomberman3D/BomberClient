#include <Global.h>
#include <Storage.h>
#include "libsqlitewrapped.h"
#include "sqlite3.h"

Database* OpenDatabase(const char* filename)
{
    char path[2048];
    sprintf(path,"%s/efcds/%s",DATA_PATH,filename);
    Database* pDB = new Database(path);

    return pDB;
}

void SQLiteQuery(Query* q, const char* query, ... )
{
    char tmp[2048];

    va_list argList;
    va_start(argList, query);
    vsprintf(tmp,query,argList);
    va_end(argList);

    q->get_result(tmp);
}

bool Storage::LoadTextureData()
{
    Database* pDB = OpenDatabase("texture.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM texture_filename");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    while (qry.fetch_row())
    {
        TextureFilename[qry.getval()] = qry.getstr();
        count++;
    }
    fprintf(stdout,"Nacteno %u odkazu textur\n",count);
    qry.free_result();

    SQLiteQuery(&qry, "SELECT * FROM texture_animation");

    if (qry.num_rows() == 0)
        return false;

    count = 0;
    uint32 textureId = 0, animId = 0, frame = 0;
    while (qry.fetch_row())
    {
        textureId = qry.getval();
        animId = qry.getval();
        frame = qry.getval();

        if (animId >= TextureAnimation[textureId].AnimFrameData.size())
            TextureAnimation[textureId].AnimFrameData.resize(animId+1);
        if (frame >= TextureAnimation[textureId].AnimFrameData[animId].size())
            TextureAnimation[textureId].AnimFrameData[animId].resize(frame+1);
        TextureAnimation[textureId].AnimFrameData[animId][frame].textureId = qry.getval();
        TextureAnimation[textureId].AnimFrameData[animId][frame].interval = qry.getval();
        count++;
    }
    fprintf(stdout,"Nacteno %u animaci textur\n",count);
    qry.free_result();

    return true;
}

bool Storage::LoadModelData()
{
    Database* pDB = OpenDatabase("model.db3");

    // Nejdriv samotne modely

    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM model_filename");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 id = 0;
    while (qry.fetch_row())
    {
        id = qry.getval();
        ModelFilename[id] = qry.getstr();
        for (uint32 i = 0; i < MAX_ANIM; i++)
        {
            ModelAnimation[id].Anim[i].frameFirst = 0;
            ModelAnimation[id].Anim[i].frameLast = 0;
            ModelAnimation[id].Anim[i].interval = 0;
        }
        count++;
    }
    fprintf(stdout,"Nacteno %u odkazu modelu\n",count);
    qry.free_result();

    // Animace

    SQLiteQuery(&qry, "SELECT * FROM model_animation");

    if (qry.num_rows() == 0)
        return false;

    count = 0;
    id = 0;
    uint32 animId = 0;
    while (qry.fetch_row())
    {
        id = qry.getval();
        animId = qry.getval();
        ModelAnimation[id].Anim[animId].frameFirst = qry.getval();
        ModelAnimation[id].Anim[animId].frameLast  = qry.getval();
        ModelAnimation[id].Anim[animId].interval   = qry.getval();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat animaci modelu\n",count);
    qry.free_result();

    return true;
}

bool Storage::LoadSkyboxData()
{
    Database* pDB = OpenDatabase("skybox.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM skybox_data");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 id;
    while (qry.fetch_row())
    {
        id = qry.getval();
        for (uint8 i = 0; i < 6; i++)
            SkyboxData[id].box_textures[i] = qry.getval();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat skyboxu\n",count);
    qry.free_result();

    return true;
}

bool Storage::LoadMapData()
{
    Database* pDB = OpenDatabase("map.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM map_data");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 id;
    while (qry.fetch_row())
    {
        id = qry.getval();
        MapData[id].filename = qry.getstr();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat pro mapy\n",count);
    qry.free_result();

    return true;
}
