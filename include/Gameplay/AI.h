#ifndef BOMB_AI_H
#define BOMB_AI_H

#include <Global.h>
#include <Map.h>
#include <Gameplay.h>

/** \struct PathNode
 *  \brief Bod cesty (pole ve 2D mape)
 */
struct PathNode
{
    uint32 x, y;
    uint8 flags;
};

/** \var PathNode::x
 *  \brief Xova souradnice bodu cesty
 */

/** \var PathNode::y
 *  \brief Yova souradnice bodu cesty
 */

/** \var PathNode::flags
 *  \brief Zatim neaktivni - priznak pro dany bod cesty, v budoucnu napriklad polozeni bomby, prepocitani cesty, atp.
 */

typedef std::vector<PathNode> Path;
typedef std::pair<uint32, uint32> CoordPair;

/** \brief Trida hledace cesty k cili
 *
 * Snazi se najit cestu ze zadanych zdrojovych souradnic ve 2D mape na zadane cilove souradnice
 */
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
        std::vector<std::vector<int8>> accessMatrixDyn;

    protected:
        Path* m_path;

        uint32 m_sourceX;
        uint32 m_sourceY; // v OGL prostoru by to bylo Z
        uint32 m_destX;
        uint32 m_destY;

        uint32 m_mapSizeX;
        uint32 m_mapSizeY;
};

/** \fn Pathfinder::Pathfinder
 *  \brief Konstruktor
 *
 * Volany s parametrem ukazatele na cestu, kterou vymaze a bude ji naplnovat v Recursoru
 */

/** \fn Pathfinder::Initialize(uint32 sourceX, uint32 sourceY, uint32 destX, uint32 destY)
 *  \brief Inicializace pathfinderu, tvorba dynamicke pristupove mapy
 *
 * Zapise potrebna data a vytvori dynamickou pristupovou mapu
 */

/** \fn Pathfinder::GeneratePath
 *  \brief Spusti rekurzivni generator cesty
 *
 * Vymaze cestu a spusti rekurzivni volani funkce Pathfinder::Recursor. Nakonec cestu prevrati, prida zdrojovou node a vrati jako hotovou
 */

/** \fn Pathfinder::Recursor(uint32 x, uint32 y)
 *  \brief Rekurzivni funkce pro generovani cesty
 *
 * Rekurzivni funkce, ktera zjisti nejlepsi smer cesty (pripadne nahodny), overi podminku, zdali to neni cilove pole, a pokud neni, provede zanoreni.
 */

/** \var Pathfinder::accessMatrixDyn
 *  \brief Dynamicka pristupova mapa
 *
 * Generuje se ve funkci Pathfinder::Initialize.
 *
 * Slouzi k determinovani pristupne cesty, kterou muzeme projit pri generovani cesty k cili (nahodne cesty, unikove cesty, ..). Plni se jednickami, kde je pristupno,
 * nulami kde neni, a dvojkami, kde uz jsme byli pri generovani cesty.
 */

/** \class OutOfZeroPathfinder
 * \brief Generator cesty z nebezpecnych poli
 *
 * Postara se o vygenerovani cesty z nebezpecnych poli na nejake bezpecne, pokud existuje
 *
 * Vice info viz dokumentace tridy Pathfinder a jejich metod
 */
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
        std::vector<std::vector<int8>> accessMatrixDyn;

    protected:
        Path* m_path;

        uint32 m_sourceX;
        uint32 m_sourceY; // v OGL prostoru by to bylo Z

        uint32 m_mapSizeX;
        uint32 m_mapSizeY;
};

/** \class RandomPathfinder
 * \brief Generator nahodne cesty vcetne vyhybani se nebezpecnym polim
 *
 * Postara se o vygenerovani nahodne cesty vcetne vyhnuti se nebezpecnym polim
 *
 * Vice info viz dokumentace tridy Pathfinder a jejich metod
 */
class RandomPathfinder
{
    public:
        RandomPathfinder(Path* path);
        ~RandomPathfinder();

        virtual void Initialize(uint32 sourceX, uint32 sourceY, uint32 length = 5);
        void GeneratePath();
        bool Recursor(uint32 x, uint32 y);

        std::vector<std::vector<int8>> accessMatrixDyn;

    protected:
        Path* m_path;

        uint32 m_sourceX;
        uint32 m_sourceY;
        uint32 m_length;
        uint32 m_curLength;

        uint32 m_mapSizeX;
        uint32 m_mapSizeY;
};

/** \var RandomPathfinder::m_length
 *  \brief Maximalni delka cesty
 *
 * Generovani cesty se ukonci, jakmile je dosazena tato delka cesty
 */

/** \class RandomBarePathfinder
 * \brief Generator nahodne cesty bez vyhybani se nebezpecnym polim
 *
 * Postara se o vygenerovani nahodne cesty po jakychkoliv pruchodnych polich
 *
 * Vice info viz dokumentace tridy Pathfinder a jejich metod
 */
class RandomBarePathfinder: public RandomPathfinder
{
    public:
        RandomBarePathfinder(Path* path): RandomPathfinder(path) {};

        void Initialize(uint32 sourceX, uint32 sourceY, uint32 length = 5);
};

// Cas na dalsi update pohyboveho generatoru [ms]
#define HOLDER_UPDATE_DELAY 500
#define MOVEMENT_DELAY_COEF 1.5f;

enum MovementType
{
    MOVEMENT_NONE      = 0,
    MOVEMENT_TARGETTED = 1,
    MOVEMENT_RANDOM    = 2,
    MOVEMENT_OUTOFZERO = 3,
    MOVEMENT_RANDOM_BARE = 4,
    MOVEMENT_MAX,
};

/** \class MovementHolder
 *  \brief Drzitel veskerych dat o pohybu nepritele
 */
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

/** \fn MovementHolder::GetMovementType
 *  \brief Vraci aktualni generator cesty
 */

/** \fn MovementHolder::SetSpeedMod(float speedMod)
 *  \brief Nastavi celkovy koeficient rychlosti pohybu
 */

/** \fn MovementHolder::GetSpeedMod
 *  \brief Vraci aktualni koeficient rychlosti pohybu
 */


/** \class EnemyTemplate
 *  \brief Predloha "umele inteligence" nepritele
 */
class EnemyTemplate
{
    public:
        EnemyTemplate() { pRecord = NULL; m_isDead = false; m_movement = new MovementHolder(this); m_nextMoveTypeUpdate = 0; m_AILevel = 1; };
        void Init(uint32 modelId, uint32 x, uint32 y, uint32 position);
        void Update();

        bool IsDead() { return m_isDead; };
        void SetDead(bool dead = true) { m_isDead = dead; };

        ModelDisplayListRecord* pRecord;

        MovementHolder* m_movement;

    protected:
        clock_t m_nextMoveTypeUpdate;
        bool m_isDead;
        uint8 m_AILevel;
        uint32 m_enemyX, m_enemyY;
};

/** \fn EnemyTemplate::EnemyTemplate
 *  \brief Konstruktor
 *
 * Opet pouze a jen nulovani promennych a ukazatelu
 */

/** \fn EnemyTemplate::IsDead
 *  \brief Vraci true, pokud je tento nepritel mrtvy (nepohybuje se, nemuze nikoho usmrtit)
 */

/** \fn EnemyTemplate::SetDead(bool dead = true)
 *  \brief Usmrti, pripadne ozivi nepritele
 */

class MemeEnemy: public EnemyTemplate
{
    public:
        MemeEnemy(): EnemyTemplate() { m_respawnTime = 0; }

    protected:
        clock_t m_respawnTime;
};

#endif
