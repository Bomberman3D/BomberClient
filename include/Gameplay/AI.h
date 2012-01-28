#ifndef BOMB_AI_H
#define BOMB_AI_H

#include <Global.h>
#include <Map.h>
#include <Gameplay.h>

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
        Pathfinder(Path* path);
        ~Pathfinder();

        void Initialize(uint32 sourceX, uint32 sourceY, uint32 destX, uint32 destY);
        void GeneratePath();
        bool Recursor(uint32 x, uint32 y);

        // Mapa, kde jsou same jednotky, vynuluji se pouze nepristupna mista (bomby, krabice,..)
        // Nuluji se jeste k tomu jiz navstivena mista pathfinderem
        std::map<CoordPair, int8> accessMatrixDyn;

    private:
        Path* m_path;

        uint32 m_sourceX;
        uint32 m_sourceY; // v OGL prostoru by to bylo Z
        uint32 m_destX;
        uint32 m_destY;

        uint32 m_mapSizeX;
        uint32 m_mapSizeY;
};

class OutOfZeroPathfinder
{
    public:
        OutOfZeroPathfinder(Path* path);
        ~OutOfZeroPathfinder();

        void Initialize(uint32 sourceX, uint32 sourceY);
        void GeneratePath();
        bool Recursor(uint32 x, uint32 y);

        // Mapa, kde jsou same jednotky, vynuluji se pouze nepristupna mista (bomby, krabice,..)
        // Nuluji se jeste k tomu jiz navstivena mista pathfinderem
        std::map<CoordPair, int8> accessMatrixDyn;

    private:
        Path* m_path;

        uint32 m_sourceX;
        uint32 m_sourceY; // v OGL prostoru by to bylo Z

        uint32 m_mapSizeX;
        uint32 m_mapSizeY;
};

class RandomPathfinder
{
    public:
        RandomPathfinder(Path* path);
        ~RandomPathfinder();

        void Initialize(uint32 sourceX, uint32 sourceY, uint32 length = 5);
        void GeneratePath();
        bool Recursor(uint32 x, uint32 y);

        std::map<CoordPair, int8> accessMatrixDyn;

    private:
        Path* m_path;

        uint32 m_sourceX;
        uint32 m_sourceY;
        uint32 m_length;
        uint32 m_curLength;

        uint32 m_mapSizeX;
        uint32 m_mapSizeY;
};

// Cas na dalsi update pohyboveho generatoru [ms]
#define HOLDER_UPDATE_DELAY 500

enum MovementType
{
    MOVEMENT_NONE      = 0,
    MOVEMENT_TARGETTED = 1,
    MOVEMENT_RANDOM    = 2,
    MOVEMENT_OUTOFZERO = 3,
    MOVEMENT_MAX,
};

class MovementHolder
{
    public:
        MovementHolder(EnemyTemplate* src);
        ~MovementHolder();

        void Mutate(MovementType moveType);
        bool TryMutate(MovementType moveType);

        bool HasPath();
        MovementType GetMovementType() { return m_moveType; };

        void SetSpeedMod(float speedMod) { m_speedMod = speedMod; };
        float GetSpeedMod() { return m_speedMod; };

        void Update();

        void Generator();
        bool TryGenerator(MovementType moveType);

    private:
        MovementType m_moveType;

        Path m_path;
        Path m_tryPath;
        uint32 m_currentPathNode;
        CVector2 m_nodeVector;
        clock_t m_nodeStartTime;
        float m_speedMod;

        clock_t m_nextUpdate;
        clock_t m_lastMoveUpdate;

        EnemyTemplate* m_src;
};

class EnemyTemplate
{
    public:
        EnemyTemplate() { pRecord = NULL; m_isDead = false; m_movement = new MovementHolder(this); m_nextMoveTypeUpdate = 0; };
        void Init(uint32 modelId, uint32 x, uint32 y);
        void Update();

        bool IsDead() { return m_isDead; };
        void SetDead(bool dead = true) { m_isDead = dead; };

        ModelDisplayListRecord* pRecord;

        MovementHolder* m_movement;

    private:
        clock_t m_nextMoveTypeUpdate;
        bool m_isDead;
};

#endif
