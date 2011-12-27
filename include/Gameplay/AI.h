#ifndef BOMB_AI_H
#define BOMB_AI_H

#include <Global.h>
#include <Map.h>

struct PathNode
{
    uint32 x, y;
    uint8 flags; // jeste nepouzito - napr. polozit bombu, prepocitat cestu, atd...
};

typedef std::vector<PathNode> Path;
typedef std::pair<uint32, uint32> CoordPair;

class Pathfinder
{
    public:
        Pathfinder();
        ~Pathfinder();

        void Initialize(uint32 sourceX, uint32 sourceY, uint32 destX, uint32 destY);
        void GeneratePath();
        bool Recursor(uint32 x, uint32 y);

        void PrintAndWait();

        Path m_path;

        // Mapa, kde jsou same jednotky, vynuluji se pouze nepristupna mista (bomby, krabice,..)
        // Nuluji se jeste k tomu jiz navstivena mista pathfinderem
        std::map<CoordPair, int8> accessMatrixDyn;

    private:
        uint32 m_sourceX;
        uint32 m_sourceY; // v OGL prostoru by to bylo Z
        uint32 m_destX;
        uint32 m_destY;

        uint32 m_mapSizeX;
        uint32 m_mapSizeY;
};

#endif
