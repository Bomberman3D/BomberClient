#ifndef BOMB_MAP_H
#define BOMB_MAP_H

#include <Global.h>
#include <Singleton.h>

#define SOLID_BOX_HEIGHT 0.7f

struct DisplayListRecord;

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
    DYNAMIC_TYPE_BONUS = 4,
};

/** \struct Chunk
 *  \brief Vychozi chunk ("bunka") mapy
 */
struct Chunk
{
    uint32 x;
    uint32 y;
    uint16 type;
    uint16 texture;
};

/** \var Chunk::x
 *  \brief Xova pozice precteneho chunku
 */

/** \var Chunk::y
 *  \brief Yova pozice precteneho chunku
 */

/** \var Chunk::type
 *  \brief Typ pole na mape na danych souradnicich
 */

/** \var Chunk::texture
 *  \brief ID textury daneho pole mapy
 */

/** \class Map
 *  \brief Holder aktualni mapy s pomocnymi funkcemi
 */
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

        /** \struct Map::Cell
         *  \brief Jedna staticka bunka mapy
         */
        struct Cell
        {
            uint32 type;
            uint32 texture;
            DisplayListRecord* pRec;
        };

        /** \struct Map::DynamicCell
         *  \brief Jedna dynamicka bunka mapy
         */
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
        bool IsDynamicRecordPresent(uint32 x, uint32 y, int32 type = -1);
        bool NearStartPos(uint32 x, uint32 y);

        uint32 GetStaticRecord(uint32 x, uint32 y);

        uint32 mapId;
        uint32 m_skybox;
        std::string m_mapName;

        MapField field;
        DynamicMapField dynfield;
};

/** \fn Map::Map
 *  \brief Konstruktor
 *
 * Pouze nulovani zaznamu
 */

/** \var Map::Cell::type
 *  \brief Typ pole na staticke mape, viz enumerator FieldType
 */

/** \var Map::Cell::texture
 *  \brief ID textury na danem poli mapy
 */

/** \var Map::Cell::pRec
 *  \brief Ukazatel na pripadny zaznam v displaylistu (v pripade modelu na te pozici)
 */

/** \var Map::DynamicCell::type
 *  \brief Typ dynamicke bunky, viz enumerator DynamicFieldType
 */

/** \var Map::DynamicCell::state
 *  \brief Puvodne mineno pro stav vybuchle bomby apod., ted zatim bez vyuziti
 */

/** \var Map::DynamicCell::misc
 *  \brief Urcuje casto typ dane dynamicke bunky, napriklad typ bonusu na tomto miste
 */

/** \var Map::DynamicCell::special
 *  \brief Ukazatel na specialni featuru dane bunky, napriklad billboard bonusu, nebo model bomby
 */


/** \class MapManager
 *  \brief Trida starajici se o spravu mapy a dat s ni spojenych
 */
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

        Map* GetMap() { return pMap; };

    private:
        void SetMapData(uint32 id);
        void SetMapFile(const char* filename);
        void SetMapId(uint32 id);

        Map* pMap;
        std::string MapFilename;
};

#define sMapManager Singleton<MapManager>::instance()

/** \fn MapManager::MapManager
 *  \brief Konstruktor
 *
 * Jen vynuluje aktualni mapu
 */

/** \fn MapManager::~MapManager
 *  \brief Destruktor
 *
 * Pokud existuje mapa, uvolni ji
 */

/** \fn MapManager::GetMap
 *  \brief Vraci ukazatel na aktualni mapu
 */

#endif
