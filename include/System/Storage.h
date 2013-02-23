#ifndef BOMB_STORAGE_H
#define BOMB_STORAGE_H

#include <Global.h>
#include <Singleton.h>
#include <Spinlock.h>
#include <Display.h>
#include <Map.h>

#include <ImageLoader.h>
#include <ModelLoader.h>

enum ModelAnimType
{
    ANIM_NONE      = 0,
    ANIM_IDLE      = 1,
    ANIM_WALK      = 2,
    ANIM_SPECIAL   = 3,
    ANIM_DISAPPEAR = 4,
    ANIM_DYING     = 5,
    MAX_ANIM
};

enum ChatMessageType
{
    CHAT_MSG_PLAYER_CHAT    = 0,
    CHAT_MSG_SERVER         = 1,
    MAX_CHAT_MSG
};

// Runtime storage

/** \struct ScoreBoardData
 *  \brief Struktura pouzivana pro uchovavani skore v multiplayer hre
 */
struct ScoreBoardData
{
    std::string nickName;
    uint32 kills;
    uint32 deaths;
};

/**
 *  \brief Radici funkce pro nasi extenzi std::listu, radi podle poctu zabiti od nejvyssiho po nejnizsi
 */
static bool SortByKillsFunction(ScoreBoardData a, ScoreBoardData b)
{
    return a.kills > b.kills;
}

/** \class ScoreBoard
 *  \brief Extenze std::listu pro ukladani dat o skore pro multiplayer hru
 */
class ScoreBoard: public std::list<ScoreBoardData>
{
    public:
        void SortByKills()
        {
            sort(SortByKillsFunction);
        };
};

enum InterThreadRequest
{
    REQUEST_STAGE_CHANGE         = 0,
    REQUEST_SUBSTAGE_CHANGE      = 1,
    REQUEST_MAP_CHANGE           = 2,
    REQUEST_GAME_TYPE_CHANGE     = 3,
    REQUEST_DYNAMIC_MAP_ELEMENT  = 4,
    REQUEST_DYNAMIC_MAP_FILL     = 5,
    REQUEST_PLAYER_ADD           = 6,
    REQUEST_PLAYER_ANIM          = 7,
    REQUEST_PLAYER_POS           = 8,
    REQUEST_PLAYER_ROTATION      = 9,
    REQUEST_PLANT_BOMB           = 10,
    REQUEST_PLAYER_DEATH         = 11,
    REQUEST_BOX_DESTROY          = 12,
    REQUEST_FILL_SCOREBOARD      = 13,
    REQUEST_RESPAWN_PLAYER       = 14,
    REQUEST_CHAT_MESSAGE         = 15,
    MAX_REQUEST
};

struct ThreadRequestDynamicElement
{
    uint32 x;
    uint32 y;
    Map::DynamicCell rec;
};

struct ThreadRequestPlayerAnim
{
    uint32 id;
    uint32 anim;
    uint32 flags;
};
struct ThreadRequestPlayerPos
{
    uint32 id;
    float x;
    float y;
    float z;
};
struct ThreadRequestPlayerRotation
{
    uint32 id;
    float rotation;
};
struct ThreadRequestPlantBomb
{
    uint32 id;
    uint32 x;
    uint32 y;
    uint32 reach;
};
struct ThreadRequestPlayerDeath
{
    uint32 id;
    float x, z;
    uint32 respawnDelay;
};
struct ThreadRequestBoxDestroy
{
    uint32 x, y;
};
struct ThreadRequestFillScoreBoard
{
    ScoreBoard scores;
};
struct ThreadRequestRespawnPlayer
{
    uint32 id;
    float x, z;
};
struct ThreadRequestChatMessage
{
    uint8 type;
    uint32 sourceId;
    std::string message;
};

// Definice jednotlivych ulozist

struct TextureData
{
    uint32 flags;
};

/** \struct Skybox
 *  \brief Struktura dat skyboxu
 */
struct Skybox
{
    uint32 box_textures[6];
};

/** \var Skybox::box_textures
 *  \brief Textury skyboxu
 *
 * V poradi: dolni, horni, zdani, predni, leva, prava
 */


/** \struct ModelAnim
 *  \brief Struktura animacnich dat modelu
 */
struct ModelAnim
{
    /** \struct ModelAnim::ModelAnimRecord
     *  \brief Struktura zaznamu jedne animace modelu
     */
    struct ModelAnimRecord
    {
        uint32 frameFirst;
        uint32 frameLast;
        uint32 interval;
    } Anim[MAX_ANIM];
};

/** \var ModelAnim::ModelAnimRecord::frameFirst
 *  \brief Prvni snimek dane animace
 */

/** \var ModelAnim::ModelAnimRecord::frameLast
 *  \brief Posledni snimek dane animace
 */

/** \var ModelAnim::ModelAnimRecord::interval
 *  \brief Interval do dalsiho snimku
 */


/** \struct TextureAnim
 *  \brief Struktura animacnich dat textur
 */
struct TextureAnim
{
    /** \struct FrameData
     *  \brief Struktura zaznamu jedne animace textury
     */
    struct FrameData
    {
        uint32 textureId;
        uint32 interval;
    };

    std::vector<std::vector<FrameData>> AnimFrameData;
};

/** \var TextureAnim::FrameData::textureId
 *  \brief ID textury na danem snimku
 */

/** \var TextureAnim::FrameData::interval
 *  \brief Interval do dalsiho snimku
 */

/** \var TextureAnim::AnimFrameData
 *  \brief Vlastni data framu - pristupovani stylem AnimFrameData[animId][frame]
 */


/** \struct MapData
 *  \brief Struktura dat mapy
 */
struct MapData
{
    std::string filename;
    std::string mapname;
};

/** \var MapData::filename
 *  \brief Cesta k souboru mapy
 */

/** \var MapData::mapname
 *  \brief Jmeno mapy zobrazovane ve hre
 */


/** \struct SolidBoxData
 *  \brief Struktura dat pevnych objektu mapy
 */
struct SolidBoxData
{
    uint32 texture_top;
    uint32 texture_sides[4];
    uint32 model_id;
};

/** \var SolidBoxData::texture_top
 *  \brief Horni textura pevne kostky
 */

/** \var SolidBoxData::texture_sides
 *  \brief Bocni textury pevne kostky
 */

/** \var SolidBoxData::model_id
 *  \brief Pokud je nastaveno (neni 0), pouzije se texture_top jako podklad a na nem se vykresli model s danym ID
 */


/** \struct ModelModifierData
 *  \brief Struktura modifikatoru pro modely
 */
struct ModelModifierData
{
    float collision_dist_mod;
};

/** \var ModelModifierData::collision_dist_mod
 *  \brief Koeficient kolizni vzdalenosti
 */


/** \struct ObjectModifierData
 *  \brief Struktura modifikatoru jednotlivych objektu
 */
struct ObjectModifierData
{
    float texture_repeat_x;
    float texture_repeat_y;
};

/** \var ObjectModifierData::texture_repeat_x
 *  \brief Koeficient opakovatelnosti textury po ose X
 */

/** \var ObjectModifierData::texture_repeat_y
 *  \brief Koeficient opakovatelnosti textury po ose Y
 */


/** \struct ObjectArtkitData
 *  \brief Struktura vzhledu artkitu pro dany objekt
 */
struct ObjectArtkitData
{
    uint32 artkit_id;

    uint8 colors[3];
};

/** \var ObjectArtkitData::artkit_id
 *  \brief ID tohoto artkitu
 */

/** \var ObjectArtkitData::colors
 *  \brief Barva daneho objektu (R, G, B) (objekt a model jsou identifikovany v poli tridy Storage)
 */


/** \struct MusicFileData
 *  \brief Struktura dat o hudebnich skladbach
 */
struct MusicFileData
{
    std::string filename;
    std::string author;
    std::string description;
};

/** \var MusicFileData::filename
 *  \brief Cesta k souboru hudby
 */

/** \var MusicFileData::author
 *  \brief Autor skladby
 */

/** \var MusicFileData::description
 *  \brief Popis, obvykle jmeno skladby
 */

#define MAX_ENEMY_MODELS 4

/** \struct EnemyTemplateData
 *  \brief Struktura dat o nepratelskych typech
 */
struct EnemyTemplateData
{
    uint32 defAILevel;
    uint32 modelID[MAX_ENEMY_MODELS];
};

/** \var EnemyTemplateData::defAILevel
 *  \brief Vychozi uroven AI
 */

/** \class Storage
 *  \brief Trida uchovavajici veskera staticka data a cast dynamickych, stara se take o jejich nacteni
 */
class Storage
{
    public:
        Storage();

        void Load();
        void PrepareDynamicStorages();

        // Helper funkce
        t3DObject* FindModelObject(uint32 modelId, const char* objectname);
        t3DObject* FindModelObjectInNonStored(t3DModel* model, const char* objectname);
        ModelAnimType GetAnimTypeForFrame(uint32 modelId, uint32 frame);

        ModelModifierData* GetModelModifierData(uint32 modelId);
        ObjectModifierData* GetObjectModifierData(uint32 modelId, const char* objectname);
        ObjectArtkitData* GetObjectArtkitData(uint32 modelId, const char* objectname, uint32 artkitId);
        void GetAllModelArtkitIds(uint32 modelId, std::vector<uint32>* dest);
        void MakeInterThreadRequest(LockThread requester, InterThreadRequest type, uint32 value);
        void MakeInterThreadObjectRequest(LockThread requester, InterThreadRequest type, void* value);

        // StorageLoader.cpp
        bool LoadTextureData();
        bool LoadModelData();
        bool LoadSkyboxData();
        bool LoadMapData();
        bool LoadMapObjectData();
        bool LoadMusicData();
        bool LoadSoundEffectData();
        bool LoadEnemyData();

        // Typedefs
        typedef std::pair<uint32, std::string> IDNamePair;

        typedef std::map<uint32, std::string> IdFilenameMap;
        typedef std::map<uint32, TextureData> TextureDataMap;
        typedef std::map<uint32, ModelAnim>   ModelAnimMap;
        typedef std::map<uint32, TextureAnim> TextureAnimMap;
        typedef std::map<uint32, Skybox>      SkyboxMap;
        typedef std::map<uint32, MapData>     MapDataMap;
        typedef std::map<uint32, SolidBoxData> SolidBoxDataMap;
        typedef std::map<uint32, ModelModifierData> ModelModifierMap;
        typedef std::map<IDNamePair, ObjectModifierData> ObjectModifierMap;
        typedef std::map<IDNamePair, std::vector<ObjectArtkitData>> ObjectArtkitMap;
        typedef std::map<uint32, MusicFileData> MusicDataMap;
        typedef std::map<uint32, EnemyTemplateData> EnemyTemplateMap;

        // Uloziste (externi, SQLite)
        IdFilenameMap TextureFilename;
        TextureDataMap TextureInfo;
        IdFilenameMap ModelFilename;
        ModelAnimMap ModelAnimation;
        TextureAnimMap TextureAnimation;
        SkyboxMap SkyboxData;
        MapDataMap MapData;
        SolidBoxDataMap SolidBoxProp;
        ModelModifierMap ModelModifiers;
        ObjectModifierMap ObjectModifiers;
        ObjectArtkitMap ObjectArtkits;
        MusicDataMap MusicData;
        IdFilenameMap SoundEffectData;
        EnemyTemplateMap EnemyData;

        // Dynamicky ukladana data
        // textury, modely a podobne
        std::map<uint32, uint32*> Textures;         // klasicke textury (podlahy, billboardy)
        std::map<uint64, uint32*> MaterialTextures; // pro modely (first ulozen jako PAIR64 model-material)
        std::map<uint32, t3DModel*> Models;         // modely

        // Runtime uloziste - instance list, nick a podobne
        std::string m_instanceList;
        std::string m_nickName;
        uint32 m_myId;
        uint32 m_myModelArtkit;
        uint32 m_instanceId;
        ScoreBoard m_scoreBoard;
        time_t m_respawnTime;
        float m_serverStartPos[2];
        bool m_respawnRequest;

        // Mezivlaknove pozadavky
        std::list<std::pair<uint32, uint32>> m_interThreadRequests;
        std::list<std::pair<uint32, void*>>  m_interThreadObjectRequests;
};

#define sStorage Singleton<Storage>::instance()

/** \var Storage::TextureFilename
 *  \brief Mapa: ID textury - jmeno souboru s texturou
 */

/** \var Storage::ModelFilename
 *  \brief Mapa: ID modelu - jmeno souboru s modelem
 */

/** \var Storage::ModelAnimation
 *  \brief Mapa: ID modelu - pole s animacnimi daty, viz struktura ModelAnim
 */

/** \var Storage::TextureAnimation
 *  \brief Mapa: ID textury - pole s animacnimi daty, viz strukura TextureAnim
 */

/** \var Storage::SkyboxData
 *  \brief Mapa: ID skyboxu - pole s ID textur skyboxu, viz struktura Skybox
 */

/** \var Storage::MapData
 *  \brief Mapa: ID mapy - struktura s detaily, viz struktura MapData
 */

/** \var Storage::SolidBoxProp
 *  \brief Mapa: ID solidboxu - struktura s detaily, viz struktura SolidBoxData
 */

/** \var Storage::ModelModifiers
 *  \brief Mapa: ID modelu - struktura s modifikatory, viz struktura ModelModifierData
 */

/** \var Storage::ObjectModifiers
 *  \brief Mapa: par ID modelu, jmeno objektu - struktura s modifikatory, viz struktura ObjectModifierData
 */

/** \var Storage::ObjectArtkits
 *  \brief Mapa: par ID modelu, jmeno objektu - pole s daty artkitu, viz struktura ObjectArtkitData
 */

/** \var Storage::MusicData
 *  \brief Mapa: ID hudby - struktura s detaily, viz struktura MusicFileData
 */

/** \var Storage::Textures
 *  \brief Dynamicke uloziste, mapa: ID textury - ukazatel do pameti na OpenGL ID textury (NULL pokud neni nactena)
 */

/** \var Storage::Models
 *  \brief Dynamicke uloziste, mapa: ID modelu - ukazatel do pameti na nacteny model (NULL pokud neni nacten)
 */

/** \var Storage::MaterialTextures
 *  \brief Dynamicke uloziste, mapa: par ID modelu, ID materialu - ukazatel do pameti na OpenGL ID textury (NULL pokud neni nactena)
 */

#endif
