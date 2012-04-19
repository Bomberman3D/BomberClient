#ifndef BOMB_STORAGE_H
#define BOMB_STORAGE_H

#include <Global.h>
#include <Singleton.h>

#include <ImageLoader.h>
#include <ModelLoader.h>

enum ModelAnimType
{
    ANIM_NONE      = 0,
    ANIM_IDLE      = 1,
    ANIM_WALK      = 2,
    ANIM_SPECIAL   = 3,
    ANIM_DISAPPEAR = 4,
    MAX_ANIM
};

// Definice jednotlivych ulozist

struct Skybox
{
    uint32 box_textures[6];
};

struct ModelAnim
{
    struct
    {
        uint32 frameFirst;
        uint32 frameLast;
        uint32 interval;
    } Anim[MAX_ANIM];
};

struct TextureAnim
{
    struct FrameData
    {
        uint32 textureId;
        uint32 interval;
    };

    // Vlastni data framu - pristupovani stylem AnimFrameData[animId][frame]
    std::vector<std::vector<FrameData>> AnimFrameData;
};

struct MapData
{
    std::string filename;
    std::string mapname;
};

struct SolidBoxData
{
    uint32 texture_top;
    uint32 texture_sides[4];
    uint32 model_id;
};

struct ModelModifierData
{
    float collision_dist_mod;
};

struct ObjectModifierData
{
    float texture_repeat_x;
    float texture_repeat_y;
};

struct ObjectArtkitData
{
    uint32 artkit_id;

    uint8 colors[3]; // R, G, B
};

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

        // StorageLoader.cpp
        bool LoadTextureData();
        bool LoadModelData();
        bool LoadSkyboxData();
        bool LoadMapData();
        bool LoadMapObjectData();

        // Typedefs
        typedef std::pair<uint32, std::string> IDNamePair;

        typedef std::map<uint32, std::string> IdFilenameMap;
        typedef std::map<uint32, ModelAnim>   ModelAnimMap;
        typedef std::map<uint32, TextureAnim> TextureAnimMap;
        typedef std::map<uint32, Skybox>      SkyboxMap;
        typedef std::map<uint32, MapData>     MapDataMap;
        typedef std::map<uint32, SolidBoxData> SolidBoxDataMap;
        typedef std::map<uint32, ModelModifierData> ModelModifierMap;
        typedef std::map<IDNamePair, ObjectModifierData> ObjectModifierMap;
        typedef std::map<IDNamePair, std::vector<ObjectArtkitData>> ObjectArtkitMap;

        // Uloziste (externi, SQLite)
        IdFilenameMap TextureFilename;
        IdFilenameMap ModelFilename;
        ModelAnimMap ModelAnimation;
        TextureAnimMap TextureAnimation;
        SkyboxMap SkyboxData;
        MapDataMap MapData;
        SolidBoxDataMap SolidBoxProp;
        ModelModifierMap ModelModifiers;
        ObjectModifierMap ObjectModifiers;
        ObjectArtkitMap ObjectArtkits;

        // Dynamicky ukladana data
        // textury, modely a podobne
        std::map<uint32, uint32*> Textures;         // klasicke textury (podlahy, billboardy)
        std::map<uint64, uint32*> MaterialTextures; // pro modely (first ulozen jako PAIR64 model-material)
        std::map<uint32, t3DModel*> Models;         // modely

        // Runtime uloziste - instance list, nick a podobne
        std::string m_instanceList;
        std::string m_nickName;
        uint32 m_myId;
        uint32 m_instanceId;
};

#define sStorage Singleton<Storage>::instance()

#endif
