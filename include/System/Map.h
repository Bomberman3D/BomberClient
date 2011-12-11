#ifndef BOMB_MAP_H
#define BOMB_MAP_H

#include <Global.h>
#include <Singleton.h>

#define SOLID_BOX_HEIGHT 0.7f

// Typy pole mapy
enum FieldType
{
    TYPE_GROUND    = 0,
    TYPE_SOLID_BOX = 1,
    TYPE_ROCK      = 2,
    TYPE_STARTLOC  = 3,
};

// Typy dynamickych poli mapy
enum DynamicFieldType
{
    DYNAMIC_TYPE_NONE  = 0,
    DYNAMIC_TYPE_BOX   = 1,
    DYNAMIC_TYPE_ENEMY = 2,
    DYNAMIC_TYPE_BOMB  = 3,
};

// Vychozi chunk z mapy
struct Chunk
{
    uint32 x;
    uint32 y;
    uint16 type;
    uint16 texture;
};

// Holder aktualni mapy s helper funkcemi
class Map
{
    public:
        Map()
        {
            // Inicilizovat mapu na rozmer 1x1
            field.resize(1);
            field[0].resize(1);

            // Inicializovat dynamickou mapu na rozmer 1x1x0
            dynfield.resize(1);
            dynfield[0].resize(1);
            dynfield[0][0].resize(0);

            m_skybox = 0;
            mapId  = 0;
            m_mapName = "UNKNOWN MAP";
        }

        struct Cell
        {
            uint32 type;
            uint32 texture;
        };

        struct DynamicCell
        {
            uint32 type;
            uint8 state;
            uint8 misc;
            void* special;
        };

        typedef std::vector<std::vector<Map::Cell>> MapField;
        typedef std::vector<Map::DynamicCell> DynamicCellSet;
        typedef std::vector<std::vector<DynamicCellSet>> DynamicMapField;

        // Prida dynamicky zaznam do mapy
        void AddDynamicCell(uint32 x, uint32 y, uint32 type, uint32 state = 0, uint32 misc = 0, void* special = NULL);
        Map::DynamicCellSet* GetDynamicCellSet(uint32 x, uint32 y);
        void DestroyDynamicRecords(uint32 x, uint32 y, int32 type = -1);
        void DestroyAllDynamicRecords();

        uint32 mapId;
        uint32 m_skybox;
        std::string m_mapName;

        MapField field;
        DynamicMapField dynfield;
};

class MapManager
{
    public:
        MapManager()
        {
            pMap = NULL;
        }
        ~MapManager()
        {
            if (pMap)
                delete pMap;
        }

        bool LoadMap(uint32 id);
        void FillDynamicRecords();

        const Map* GetMap() { return (const Map*)pMap; };

    private:
        void SetMapData(uint32 id);
        void SetMapFile(const char* filename);
        void SetMapId(uint32 id);

        Map* pMap;
        std::string MapFilename;
};

#define sMapManager Singleton<MapManager>::instance()

#endif
