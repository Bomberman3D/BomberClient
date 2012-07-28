#include <Global.h>
#include <Storage.h>
#include "libsqlitewrapped.h"
#include "sqlite3.h"

/** \brief Poskytuje ukazatel na otevrenou SQLite databazi
 */
Database* OpenDatabase(const char* filename)
{
    char path[2048];
    sprintf(path,"%s/efcds/%s",DATA_PATH,filename);
    Database* pDB = new Database(path);

    return pDB;
}

/** \brief Provede dotaz do dane databaze
 */
void SQLiteQuery(Query* q, const char* query, ... )
{
    char tmp[2048];

    va_list argList;
    va_start(argList, query);
    vsprintf(tmp,query,argList);
    va_end(argList);

    q->get_result(tmp);
}

/** \brief Nacteni dat textur
 */
bool Storage::LoadTextureData()
{
    Database* pDB = OpenDatabase("texture.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM texture_filename");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 textureId;
    while (qry.fetch_row())
    {
        textureId = qry.getval();
        TextureFilename[textureId] = qry.getstr();
        TextureInfo[textureId].flags = qry.getval();
        count++;
    }
    fprintf(stdout,"Nacteno %u odkazu textur\n",count);
    qry.free_result();

    SQLiteQuery(&qry, "SELECT * FROM texture_animation");

    if (qry.num_rows() == 0)
        return false;

    count = 0;
    textureId = 0;
    uint32 animId = 0, frame = 0;
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

/** \brief Nacteni dat modelu (vcetne animaci, modifikatoru a tak podobne)
 */
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

    // Specialni modifikace pro modely

    SQLiteQuery(&qry, "SELECT * FROM model_modifiers");

    if (qry.num_rows() == 0)
        return false;

    count = 0;
    id = 0;
    while (qry.fetch_row())
    {
        id = qry.getval();

        ModelModifiers[id].collision_dist_mod = qry.getnum();
        count++;
    }
    fprintf(stdout,"Nacteno %u modifikatoru modelu\n",count);
    qry.free_result();

    // Specialni modifikace pro objekty

    SQLiteQuery(&qry, "SELECT * FROM object_modifiers");

    if (qry.num_rows() == 0)
        return false;

    count = 0;
    id = 0;
    std::string name;
    IDNamePair index;
    while (qry.fetch_row())
    {
        id = qry.getval();
        name = qry.getstr();
        index = std::make_pair(id, name.c_str());

        ObjectModifiers[index].texture_repeat_x = qry.getnum();
        ObjectModifiers[index].texture_repeat_y = qry.getnum();
        count++;
    }
    fprintf(stdout,"Nacteno %u objektovych modifikatoru\n",count);
    qry.free_result();

    // Artkity pro objekty ("barevne prevleky")

    SQLiteQuery(&qry, "SELECT * FROM object_artkit");

    if (qry.num_rows() == 0)
        return false;

    count = 0;
    id = 0;
    uint32 modelid = 0;
    ObjectArtkitData tmp;
    while (qry.fetch_row())
    {
        id = qry.getval();
        modelid = qry.getval();
        name = qry.getstr();
        index = std::make_pair(modelid, name.c_str());

        tmp.artkit_id = id;

        for (uint32 i = 0; i < 3; i++)
            tmp.colors[i] = qry.getnum();

        ObjectArtkits[index].push_back(tmp);
        count++;
    }
    fprintf(stdout,"Nacteno %u artkitu pro modely\n",count);
    qry.free_result();

    return true;
}

/** \brief Nacteni data skyboxu map
 */
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

/** \brief Nacteni dat o mapach
 */
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
        MapData[id].mapname = qry.getstr();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat pro mapy\n",count);
    qry.free_result();

    return true;
}

/** \brief Nacteni dat o objektech map
 */
bool Storage::LoadMapObjectData()
{
    Database* pDB = OpenDatabase("mapobjects.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM solid_box");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 id;
    while (qry.fetch_row())
    {
        id = qry.getval();
        SolidBoxProp[id].texture_top = qry.getval();
        for (uint8 i = 0; i < 4; i++)
            SolidBoxProp[id].texture_sides[i] = qry.getval();
        SolidBoxProp[id].model_id = qry.getval();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat pro objekty mapy\n",count);
    qry.free_result();

    return true;
}

/** \brief Nacteni dat o hudebnich skladbach
 */
bool Storage::LoadMusicData()
{
    Database* pDB = OpenDatabase("music.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM music_filename");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 id;
    while (qry.fetch_row())
    {
        id = qry.getval();
        MusicData[id].filename = qry.getstr();
        MusicData[id].author = qry.getstr();
        MusicData[id].description = qry.getstr();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat hudby\n",count);
    qry.free_result();

    return true;
}

/** \brief Nacteni dat o hudebnich skladbach
 */
bool Storage::LoadSoundEffectData()
{
    Database* pDB = OpenDatabase("soundeffects.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM sound_filename");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 id;
    while (qry.fetch_row())
    {
        id = qry.getval();
        SoundEffectData[id] = qry.getstr();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat zvukovych efektu\n",count);
    qry.free_result();

    return true;
}

/** \brief Nacteni dat o nepratelich
 */
bool Storage::LoadEnemyData()
{
    Database* pDB = OpenDatabase("enemy.db3");
    Query qry(*pDB);
    SQLiteQuery(&qry, "SELECT * FROM enemy_template");

    if (qry.num_rows() == 0)
        return false;

    uint32 count = 0;
    uint32 id;
    while (qry.fetch_row())
    {
        id = qry.getval();
        EnemyData[id].defAILevel = qry.getval();
        for (uint32 i = 0; i < MAX_ENEMY_MODELS; i++)
            EnemyData[id].modelID[i] = qry.getval();
        count++;
    }
    fprintf(stdout,"Nacteno %u dat nepratel\n",count);
    qry.free_result();

    return true;
}
