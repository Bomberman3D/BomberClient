#include <Global.h>
#include <AI.h>
#include <Map.h>
#include <Display.h>
#include <Effects/Animations.h>

#include <algorithm>

/********************************************************************/
/*                                                                  */
/*               Targetted pathfinder                               */
/*                                                                  */
/********************************************************************/

Pathfinder::Pathfinder(Path* path)
{
    m_sourceX = 0;
    m_sourceY = 0;
    m_destX = 0;
    m_destY = 0;

    m_path = path;

    m_path->clear();
}

Pathfinder::~Pathfinder()
{
}

void Pathfinder::Initialize(uint32 sourceX, uint32 sourceY, uint32 destX, uint32 destY)
{
    // Inicializace vstupnich dat
    m_sourceX = sourceX;
    m_sourceY = sourceY;
    m_destX = destX;
    m_destY = destY;

    m_path->clear();

    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    // Naplneni pomocnych promennych
    m_mapSizeX = pMap->field.size();
    m_mapSizeY = pMap->field[0].size();

    accessMatrixDyn.clear();

    // "Pristupova mapa" se naplni jednickami tam, kde pathfinder muze hledat cestu
    // a nulami tam, kam nemuze
    accessMatrixDyn.resize(m_mapSizeX);
    for (uint32 i = 0; i < m_mapSizeX; i++)
    {
        accessMatrixDyn[i].resize(m_mapSizeY);

        for (uint32 j = 0; j < m_mapSizeY; j++)
        {
            // Pokud je nepristupne pole na souradnicich [x,y]
            if (pMap->field[i][j].type == TYPE_SOLID_BOX
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOX)
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOMB))
            {
                // Vynulujeme ho v hledaci mape
                accessMatrixDyn[i][j] = 0;
                continue;
            }
            else if (sGameplayMgr->WouldBeDangerousField(i,j) || sGameplayMgr->IsDangerousField(i,j))
            {
                accessMatrixDyn[i][j] = 0;
                continue;
            }

            // Pokud je pristupne, jednicku
            accessMatrixDyn[i][j] = 1;
        }
    }
}

// Makra pro rekurzivni postup po poli
// ve zkratce jen volaji dalsi zanoreni funkce Recursor a pri uspechu (true) zapisou bod do cesty
// Nastaveni na 9 je zde jen kvuli pripadnemu debugu a vystupu
#define RECURSE_RIGHT   if ((x+1) < m_mapSizeX && accessMatrixDyn[x+1][y] == 1) \
                        {                         \
                            if (Recursor(x+1, y)) \
                            {                     \
                                p->x = x+1;       \
                                p->y = y;         \
                                p->flags = 0;     \
                                accessMatrixDyn[x+1][y] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

#define RECURSE_DOWN    if ((y+1) < m_mapSizeY && accessMatrixDyn[x][y+1] == 1) \
                        {                         \
                            if (Recursor(x, y+1)) \
                            {                     \
                                p->x = x;         \
                                p->y = y+1;       \
                                p->flags = 0;     \
                                accessMatrixDyn[x][y+1] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

#define RECURSE_LEFT    if (x > 0 && accessMatrixDyn[x-1][y] == 1) \
                        {                         \
                            if (Recursor(x-1, y)) \
                            {                     \
                                p->x = x-1;       \
                                p->y = y;         \
                                p->flags = 0;     \
                                accessMatrixDyn[x-1][y] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

#define RECURSE_UP      if (y > 0 && accessMatrixDyn[x][y-1] == 1) \
                        {                         \
                            if (Recursor(x, y-1)) \
                            {                     \
                                p->x = x;         \
                                p->y = y-1;       \
                                p->flags = 0;     \
                                accessMatrixDyn[x][y-1] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

bool Pathfinder::Recursor(uint32 x, uint32 y)
{
    // pozdeji zmenit na == 0, ted jen pro nazornost se budou nastavovat dvojky jako prosla cesta
    if (accessMatrixDyn[x][y] != 1)
        return false;

    // Automaticky nastavime pole jako "navstivene"
    accessMatrixDyn[x][y] = 2;

    // Overeni, zdali se jedna o cilove pole. Pokud ano, vratime true a cela rekurze se pote vrati a cesta se zapise
    if (x == m_destX && y == m_destY)
        return true;

    // Minimalizujeme pocet lokalnich promennych kvuli velikosti zasobniku
    PathNode* p = new PathNode;

    // Uprednostni se smer, ktery je potreba "uspokojit". Pokud je cil o deset poli nalevo
    // a o pet poli nahoru, uspokoji se smer doleva a nasledne nahoru.

    // Pokud je potreba uspokojit Xovy smer
    if (abs(int(x)-int(m_destX)) > abs(int(y)-int(m_destY)))
    {
        if (x > m_destX)
        {
            RECURSE_LEFT;
            if (y > m_destY)
            {
                RECURSE_UP;
                RECURSE_DOWN;
            }
            else
            {
                RECURSE_DOWN;
                RECURSE_UP;
            }
            RECURSE_RIGHT;
        }
        else
        {
            RECURSE_RIGHT;
            if (y > m_destY)
            {
                RECURSE_UP;
                RECURSE_DOWN;
            }
            else
            {
                RECURSE_DOWN;
                RECURSE_UP;
            }
            RECURSE_LEFT;
        }
    }
    else
    {
        if (y > m_destY)
        {
            RECURSE_UP;
            if (x > m_destX)
            {
                RECURSE_LEFT;
                RECURSE_RIGHT;
            }
            else
            {
                RECURSE_RIGHT;
                RECURSE_LEFT;
            }
            RECURSE_DOWN;
        }
        else
        {
            RECURSE_DOWN;
            if (x > m_destX)
            {
                RECURSE_LEFT;
                RECURSE_RIGHT;
            }
            else
            {
                RECURSE_RIGHT;
                RECURSE_LEFT;
            }
            RECURSE_UP;
        }
    }

    return false;
}

void Pathfinder::GeneratePath()
{
    // Pokud stojime na nepristupnem poli, jsme uvezneni
    if (accessMatrixDyn[m_sourceX][m_sourceY] == 0)
        return;

    if (!Recursor(m_sourceX, m_sourceY))
    {
        // Cesta se nevygenerovala, nutno nasadit nahodny pohyb (viz. EnemyTemplate::Update)
    }
    else
    {
        // Cesta se nasla, muzeme se pohybovat po ceste

        // Vlozime i zdrojovou node
        PathNode node;
        node.x = m_sourceX;
        node.y = m_sourceY;
        node.flags = 0;
        m_path->push_back(node);

        // Ovsem.. vygenerovala se v opacnem poradi. Prevratime ji tedy
        std::reverse(m_path->begin(), m_path->end());
    }
}

/********************************************************************/
/*                                                                  */
/*               Targetted Out Of Unacessible pathfinder            */
/*                                                                  */
/********************************************************************/

OutOfZeroPathfinder::OutOfZeroPathfinder(Path* path)
{
    m_sourceX = 0;
    m_sourceY = 0;

    m_path = path;

    m_path->clear();
}

OutOfZeroPathfinder::~OutOfZeroPathfinder()
{
}

void OutOfZeroPathfinder::Initialize(uint32 sourceX, uint32 sourceY)
{
    // Inicializace vstupnich dat
    m_sourceX = sourceX;
    m_sourceY = sourceY;

    m_path->clear();

    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    // Naplneni pomocnych promennych
    m_mapSizeX = pMap->field.size();
    m_mapSizeY = pMap->field[0].size();

    accessMatrixDyn.clear();

    // "Pristupova mapa" se naplni nulami tam, kde pathfinder muze hledat cestu
    // a trojkami tam, kam nemuze (jakekoliv pole s 1 je cilove)
    accessMatrixDyn.resize(m_mapSizeX);
    for (uint32 i = 0; i < m_mapSizeX; i++)
    {
        accessMatrixDyn[i].resize(m_mapSizeY);

        for (uint32 j = 0; j < m_mapSizeY; j++)
        {
            // Podminky na pristupnost / nepristupnost pole na souradnicich [x,y]
            if (pMap->field[i][j].type == TYPE_SOLID_BOX
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOMB)
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOX))
            {
                // At stojime kdekoliv, vzdycky muzeme z toho mista odejit
                // napriklad pokud nam pod zadek byla polozena bomba
                if (i == sourceX && j == sourceY)
                    accessMatrixDyn[i][j] = 0;
                else
                    accessMatrixDyn[i][j] = 3;
                continue;
            }
            else if (sGameplayMgr->WouldBeDangerousField(i,j))
            {
                accessMatrixDyn[i][j] = 0;
                continue;
            }

            // Pokud je pristupne, jednicku
            accessMatrixDyn[i][j] = 1;
        }
    }
}

// Makra pro rekurzivni postup po poli
// ve zkratce jen volaji dalsi zanoreni funkce Recursor a pri uspechu (true) zapisou bod do cesty
// Nastaveni na 9 je zde jen kvuli pripadnemu debugu a vystupu
#define OOZ_RECURSE_RIGHT   if ((x+1) < m_mapSizeX && (accessMatrixDyn[x+1][y] == 0 || accessMatrixDyn[x+1][y] == 1)) \
                            {                         \
                                if (Recursor(x+1, y)) \
                                {                     \
                                    p->x = x+1;       \
                                    p->y = y;         \
                                    p->flags = 0;     \
                                    accessMatrixDyn[x+1][y] = 9; \
                                    m_path->push_back(*p); \
                                    return true;      \
                                }                     \
                            }

#define OOZ_RECURSE_DOWN    if ((y+1) < m_mapSizeY && (accessMatrixDyn[x][y+1] == 0 || accessMatrixDyn[x][y+1] == 1)) \
                            {                         \
                                if (Recursor(x, y+1)) \
                                {                     \
                                    p->x = x;         \
                                    p->y = y+1;       \
                                    p->flags = 0;     \
                                    accessMatrixDyn[x][y+1] = 9; \
                                    m_path->push_back(*p); \
                                    return true;      \
                                }                     \
                            }

#define OOZ_RECURSE_LEFT    if (x > 0 && (accessMatrixDyn[x-1][y] == 0 || accessMatrixDyn[x-1][y] == 1)) \
                            {                         \
                                if (Recursor(x-1, y)) \
                                {                     \
                                    p->x = x-1;       \
                                    p->y = y;         \
                                    p->flags = 0;     \
                                    accessMatrixDyn[x-1][y] = 9; \
                                    m_path->push_back(*p); \
                                    return true;      \
                                }                     \
                            }

#define OOZ_RECURSE_UP      if (y > 0 && (accessMatrixDyn[x][y-1] == 0 || accessMatrixDyn[x][y-1] == 1)) \
                            {                         \
                                if (Recursor(x, y-1)) \
                                {                     \
                                    p->x = x;         \
                                    p->y = y-1;       \
                                    p->flags = 0;     \
                                    accessMatrixDyn[x][y-1] = 9; \
                                    m_path->push_back(*p); \
                                    return true;      \
                                }                     \
                            }

bool OutOfZeroPathfinder::Recursor(uint32 x, uint32 y)
{
    // trojky jsou jako pevna pole - nejde pres ne projit
    if (accessMatrixDyn[x][y] == 2 || accessMatrixDyn[x][y] == 3)
        return false;

    // Overeni, zdali se jedna o cilove pole. Pokud ano, vratime true a cela rekurze se pote vrati a cesta se zapise
    if (accessMatrixDyn[x][y] == 1)
        return true;

    // Automaticky nastavime pole jako "navstivene"
    accessMatrixDyn[x][y] = 2;

    // Minimalizujeme pocet lokalnich promennych kvuli velikosti zasobniku
    PathNode* p = new PathNode;

    OOZ_RECURSE_LEFT;
    OOZ_RECURSE_RIGHT;
    OOZ_RECURSE_UP;
    OOZ_RECURSE_DOWN;

    return false;
}

void OutOfZeroPathfinder::GeneratePath()
{
    // Pokud stojime na pristupnem poli, jsme v suchu
    if (accessMatrixDyn[m_sourceX][m_sourceY] == 1)
        return;

    if (!Recursor(m_sourceX, m_sourceY))
    {
        // Cesta se nevygenerovala, nejspis budeme stat na miste a cekat na smrt - neni uniku
    }
    else
    {
        // Cesta se nasla, muzeme se pohybovat po ceste

        // Vlozime i zdrojovou node
        PathNode node;
        node.x = m_sourceX;
        node.y = m_sourceY;
        node.flags = 0;
        m_path->push_back(node);

        // Ovsem.. vygenerovala se v opacnem poradi. Prevratime ji tedy
        std::reverse(m_path->begin(), m_path->end());
    }
}

/********************************************************************/
/*                                                                  */
/*               Random pathfinder                                  */
/*                                                                  */
/********************************************************************/

RandomPathfinder::RandomPathfinder(Path *path)
{
    m_sourceX = 0;
    m_sourceY = 0;
    m_length = 0;

    m_path = path;

    m_path->clear();
}

RandomPathfinder::~RandomPathfinder()
{
}

void RandomPathfinder::Initialize(uint32 sourceX, uint32 sourceY, uint32 length)
{
    // Vstupni parametr length defaultne nastaven na 5

    m_sourceX = sourceX;
    m_sourceY = sourceY;
    m_length = length;

    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    // Naplneni pomocnych promennych
    m_mapSizeX = pMap->field.size();
    m_mapSizeY = pMap->field[0].size();

    accessMatrixDyn.clear();

    // "Pristupova mapa" se naplni jednickami tam, kde pathfinder muze hledat cestu
    // a nulami tam, kam nemuze
    accessMatrixDyn.resize(m_mapSizeX);
    for (uint32 i = 0; i < m_mapSizeX; i++)
    {
        accessMatrixDyn[i].resize(m_mapSizeY);

        for (uint32 j = 0; j < m_mapSizeY; j++)
        {
            // Pokud je nepristupne pole na souradnicich [x,y]
            if (pMap->field[i][j].type == TYPE_SOLID_BOX
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOX)
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOMB))
            {
                // Vynulujeme ho v hledaci mape
                accessMatrixDyn[i][j] = 0;
                continue;
            }
            else if (sGameplayMgr->WouldBeDangerousField(i,j))
            {
                accessMatrixDyn[i][j] = 0;
                continue;
            }

            // Pokud je pristupne, jednicku
            accessMatrixDyn[i][j] = 1;
        }
    }
}

bool RandomPathfinder::Recursor(uint32 x, uint32 y)
{
    if (m_curLength >= m_length)
        return true;

    m_curLength++;

    PathNode* p = new PathNode;

    // U nahodneho generatoru cesty je nam vlastne jedno, jakou cestou se vydame
    // jde nam jen o to, abychom se nedostali na spatne misto (nastavene na nulu v
    // pristupove mape)
    // Proto muzeme zvolit nahodne zanoreni, ktere je omezeno "maximalni delkou"
    switch (rand()%4)
    {
        case 0:
            RECURSE_UP;
            RECURSE_LEFT;
            RECURSE_RIGHT;
            RECURSE_DOWN;
            break;
        case 1:
            RECURSE_LEFT;
            RECURSE_RIGHT;
            RECURSE_DOWN;
            RECURSE_UP;
            break;
        case 2:
            RECURSE_RIGHT;
            RECURSE_DOWN;
            RECURSE_UP;
            RECURSE_LEFT;
            break;
        case 3:
        default:
            RECURSE_LEFT;
            RECURSE_UP;
            RECURSE_DOWN;
            RECURSE_RIGHT;
            break;
    }

    return false;
}

void RandomPathfinder::GeneratePath()
{
    m_curLength = 0;

    // Pokud stojime na nepristupnem poli, jsme uvezneni
    if (accessMatrixDyn[m_sourceX][m_sourceY] == 0)
        return;

    if (!Recursor(m_sourceX, m_sourceY))
    {
        // Cesta nenalezena
    }
    else
    {
        // Cesta nalezena

        PathNode node;
        node.x = m_sourceX;
        node.y = m_sourceY;
        node.flags = 0;
        m_path->push_back(node);

        // Opet prevratime
        std::reverse(m_path->begin(), m_path->end());
    }
}

/********************************************************************/
/*                                                                  */
/*               Random  Bare pathfinder                            */
/*                                                                  */
/********************************************************************/

void RandomBarePathfinder::Initialize(uint32 sourceX, uint32 sourceY, uint32 length)
{
    // Vstupni parametr length defaultne nastaven na 5

    m_sourceX = sourceX;
    m_sourceY = sourceY;
    m_length = length;

    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    // Naplneni pomocnych promennych
    m_mapSizeX = pMap->field.size();
    m_mapSizeY = pMap->field[0].size();

    accessMatrixDyn.clear();

    // "Pristupova mapa" se naplni jednickami tam, kde pathfinder muze hledat cestu
    // a nulami tam, kam nemuze
    accessMatrixDyn.resize(m_mapSizeX);
    for (uint32 i = 0; i < m_mapSizeX; i++)
    {
        accessMatrixDyn[i].resize(m_mapSizeY);

        for (uint32 j = 0; j < m_mapSizeY; j++)
        {
            // Pokud je nepristupne pole na souradnicich [x,y]
            if (pMap->field[i][j].type == TYPE_SOLID_BOX
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOX)
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOMB))
            {
                // Vynulujeme ho v hledaci mape
                accessMatrixDyn[i][j] = 0;
                continue;
            }

            // Pokud je pristupne, jednicku
            accessMatrixDyn[i][j] = 1;
        }
    }
}

/********************************************************************/
/*                                                                  */
/*               Movement holder class                              */
/*                                                                  */
/********************************************************************/


/** \brief Konstruktor
 *
 * Pouze nulovani hodnot a cesty
 */
MovementHolder::MovementHolder(EnemyTemplate* src)
{
    // Inicializace tridy starajici se o pohyb

    m_path.clear();
    m_tryPath.clear();
    m_src = src;
    m_moveType = MOVEMENT_NONE;
    m_lastMoveUpdate = clock();
    m_nodeStartTime = 0;

    m_speedMod = 1.0f;
}

/** \brief Destruktor
 *
 * Prazdne
 */
MovementHolder::~MovementHolder()
{
}

/** \brief Generovani cesty na jistotu
 *
 * Vygeneruje cestu za predem znameho predpokladu, ze existuje
 */
void MovementHolder::Generator()
{
    // Generator slouzi k vygenerovani cesty, o ktere jsme si jisti, ze existuje,
    // nebo k implicitnimu nastaveni
    switch (m_moveType)
    {
        case MOVEMENT_TARGETTED:
        {
            // Pokud se pohybujeme za cilem, musi cil existovat
            ModelDisplayListRecord* pTarget = sDisplay->GetTargetModel();
            if (!pTarget)
            {
                m_path.clear();
                m_moveType = MOVEMENT_NONE;
                break;
            }

            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            uint32 tx = ceil(pTarget->x);
            uint32 ty = ceil(pTarget->z);

            Path* tmpPath = new Path;
            Pathfinder p(tmpPath);
            p.Initialize(mx, my, tx, ty);
            p.GeneratePath();

            // Pouze pokud existuje kratsi cesta k cili, pouzijeme ji
            if (tmpPath->size() < m_path.size())
            {
                m_path.clear();
                m_path.assign(tmpPath->begin(), tmpPath->end());

                // Musi byt vetsi nez 1, aby melo smysl se pohybovat
                if (m_path.size() > 1)
                {
                    m_nodeVector.x =  int32(m_path[1].x) - int32(m_path[0].x);
                    m_nodeVector.y =  int32(m_path[1].y) - int32(m_path[0].y);
                    m_currentPathNode = 0;
                    m_nodeStartTime = clock();
                }
            }
            delete tmpPath;

            break;
        }
        case MOVEMENT_OUTOFZERO:
        {
            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            OutOfZeroPathfinder p(&m_path);
            p.Initialize(mx, my);
            p.GeneratePath();

            // Musi byt vetsi nez 1, aby melo smysl se pohybovat
            if (m_path.size() > 1)
            {
                m_nodeVector.x =  int32(m_path[1].x) - int32(m_path[0].x);
                m_nodeVector.y =  int32(m_path[1].y) - int32(m_path[0].y);
                m_currentPathNode = 0;
                m_nodeStartTime = clock();
            }
            break;
        }
        case MOVEMENT_RANDOM:
        {
            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            RandomPathfinder p(&m_path);
            p.Initialize(mx, my);
            p.GeneratePath();

            if (m_path.size() > 1)
            {
                m_nodeVector.x = int32(m_path[1].x) - int32(m_path[0].x);
                m_nodeVector.y = int32(m_path[1].y) - int32(m_path[0].y);
                m_currentPathNode = 0;
                m_nodeStartTime = clock();
            }
            break;
        }
        case MOVEMENT_RANDOM_BARE:
        {
            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            RandomBarePathfinder p(&m_path);
            p.Initialize(mx, my);
            p.GeneratePath();

            if (m_path.size() > 1)
            {
                m_nodeVector.x = int32(m_path[1].x) - int32(m_path[0].x);
                m_nodeVector.y = int32(m_path[1].y) - int32(m_path[0].y);
                m_currentPathNode = 0;
                m_nodeStartTime = clock();
            }
            break;
        }
    }
}

/** \brief Generuje cestu i bez jistoty, ze existuje
 *
 * Slouzi k vygenerovani cesty k cili, ale do docasne vedlejsi mapy, 
 * pote je overeno funkci TryMutate, zdali doslo k vygenerovani a pripadne se pouzije
 * vygenerovana cesta
 */
bool MovementHolder::TryGenerator(MovementType moveType)
{
    switch (moveType)
    {
        case MOVEMENT_TARGETTED:
        {
            ModelDisplayListRecord* pTarget = sDisplay->GetTargetModel();
            if (!pTarget)
            {
                m_moveType = MOVEMENT_NONE;
                break;
            }

            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            uint32 tx = ceil(pTarget->x);
            uint32 ty = ceil(pTarget->z);
            Pathfinder p(&m_tryPath);
            p.Initialize(mx, my, tx, ty);
            p.GeneratePath();

            if (m_tryPath.size() > 1)
                return true;

            break;
        }
        case MOVEMENT_OUTOFZERO:
        {
            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            OutOfZeroPathfinder p(&m_tryPath);
            p.Initialize(mx, my);
            p.GeneratePath();

            if (m_tryPath.size() > 1)
                return true;

            break;
        }
        case MOVEMENT_RANDOM:
        {
            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            RandomPathfinder p(&m_tryPath);
            p.Initialize(mx, my);
            p.GeneratePath();

            if (m_tryPath.size() > 1)
                return true;

            break;
        }
        case MOVEMENT_RANDOM_BARE:
        {
            uint32 mx = ceil(m_src->pRecord->x);
            uint32 my = ceil(m_src->pRecord->z);
            RandomBarePathfinder p(&m_tryPath);
            p.Initialize(mx, my);
            p.GeneratePath();

            if (m_tryPath.size() > 1)
                return true;

            break;
        }
    }

    return false;
}

/** \brief Nastaveni implicitniho generatoru cesty
 *
 * Pripadne se pouziva k nastaveni "na jistotu", nebo kdyz to jinak neni mozne
 */
void MovementHolder::Mutate(MovementType moveType)
{
    if (moveType >= MOVEMENT_MAX)
        return;

    if (moveType == MOVEMENT_NONE)
    {
        m_moveType = moveType;
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_IDLE, 0, 5);
        m_path.clear();
        return;
    }

    m_moveType = moveType;
    Generator();

    // Zmenime animaci modelu - je treba presunout asi, tady se mi to nelibi
    if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_WALK)
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 0);
}

/** \brief Pokusi se o zmenu movement generatoru, pokud existuje cesta
 *
 * Pokud existuje cesta, kterou by vygeneroval dany generator, aplikuje se. Jinak zustane stejny
 */
bool MovementHolder::TryMutate(MovementType moveType)
{
    if (TryGenerator(moveType))
    {
        m_moveType = moveType;
        m_path.clear();
        m_path.assign(m_tryPath.begin(), m_tryPath.end());

        m_nodeVector.x =  int32(m_path[1].x) - int32(m_path[0].x);
        m_nodeVector.y =  int32(m_path[1].y) - int32(m_path[0].y);
        m_currentPathNode = 0;
        m_nodeStartTime = clock();

        // Zmenime animaci modelu - je treba presunout asi, tady se mi to nelibi
        if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_WALK)
            sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 0);

        return true;
    }

    return false;
}

/** \brief Update funkce
 *
 * Zde se ridi pohyb po vektoru "z pole na pole", zmena animace pri pohybu, prepocitavani cesty a tak dale.
 */
void MovementHolder::Update()
{
    // Update movement holderu

    clock_t tnow = clock();

    // Pokud je cesta mensi nez 2 (tedy posun jen na pole na kterem jsme),
    // je to zbytecne a pockame tedy
    if (m_path.size() < 2)
    {
        // Zastaveni animace modelu
        if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_IDLE)
            sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_IDLE, 0, 5);

        if (tnow < m_nextUpdate)
            return;

        Generator();

        m_nextUpdate = tnow + HOLDER_UPDATE_DELAY * m_speedMod;
        return;
    }

    // natoceni modelu podle smeru kterym jdeme
    if (m_nodeVector.x > 0.0f)
        m_src->pRecord->rotate = 90.0f;
    else if (m_nodeVector.x < 0.0f)
        m_src->pRecord->rotate = 270.0f;
    else if (m_nodeVector.y > 0.0f)
        m_src->pRecord->rotate = 0.0f;
    else if (m_nodeVector.y < 0.0f)
        m_src->pRecord->rotate = 180.0f;

    // Opet zmena animace modelu na chuzi, tady se mi to taky nelibi, bude treba vymyslet nejake hezci umisteni
    if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_WALK)
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 0);

    float timePass = 500 * m_speedMod;
    float timeDiff = tnow-m_nodeStartTime;
    if (timeDiff > timePass)
        timeDiff = timePass;

    // Posun modelu po vektoru pohybu
    m_src->pRecord->x = m_path[m_currentPathNode].x - 0.5f + ( m_nodeVector.x*( timeDiff/timePass ));
    m_src->pRecord->z = m_path[m_currentPathNode].y - 0.5f + ( m_nodeVector.y*( timeDiff/timePass ));

    // No a pokud jsme dosahli maximalniho bodu usecky urcene vektorem a delkou 1,
    // posuneme se na dalsi bod cesty
    if (tnow-m_nodeStartTime >= timePass)
    {
        m_currentPathNode++;
        if (m_currentPathNode >= m_path.size()-1)
        {
            // Dostali jsme se na konec cesty, nechame si vygenerovat novou cestu
            m_currentPathNode--;
            Generator();
        }
        else
        {
            // existence node+1 zabezpecena v checku vyse
            m_nodeVector.x = int32(m_path[m_currentPathNode+1].x) - int32(m_path[m_currentPathNode].x);
            m_nodeVector.y = int32(m_path[m_currentPathNode+1].y) - int32(m_path[m_currentPathNode].y);

            m_nodeStartTime = tnow;
            if (m_moveType == MOVEMENT_TARGETTED)
                Generator();
        }
    }
}

/** \brief Zjisteni, zdali existuje cesta
 */
bool MovementHolder::HasPath()
{
    return (m_path.size() > 1);
}

/** \brief Inicializace nepritele
 *
 * Zde dojde k vytvoreni modelu, jeho featur, zvetseni/zmenseni a podobne. Zaroven se zde nastavuje uroven jeho inteligence pomoci udaju zadanych hracem
 */
void EnemyTemplate::Init(uint32 modelId, uint32 x, uint32 y, uint32 position)
{
    m_AILevel = (uint8)sGameplayMgr->GetSetting(SETTING_ENEMY_AI_LEVEL);

    float scale = 1.0f;
    float height = 0.0f;
    uint32 artkit = 0;
    // Nastaveni atributu podle modelu
    switch (modelId)
    {
        case 1:
            scale = 0.2f;
            break;
        case 10:
            scale = 0.8f;
            height = 0.2f;
            break;
        case 11:
            scale = 3.5f;
            // Mame pouze 3 artkity pro tenhle model
            if (position < 4)
                artkit = position;
            else
                artkit = 3;
            break;
        case 12:
            scale = 0.07f;
            break;
    }

    // Inicializace modelu a zaznamu modelu pro nepritele
    pRecord = sDisplay->DrawModel(modelId, x-0.5f, height, y-0.5f, ANIM_IDLE, scale, 0.0f, true, false, 0, 0, ANIM_RESTRICTION_NOT_PAUSED, false, artkit);

    // Rutiny po pridani (implicitni featury modelu,...)
    switch (modelId)
    {
        case 10:
            sDisplay->AddModelFeature(pRecord, MF_TYPE_EMITTER, 0.0f, 2.0f, 0.0f, sParticleEmitterMgr->AddEmitter(
                BillboardDisplayListRecord::Create(61, 0,0,0, 0.2f, 0.2f, true, true), 0,0,0,0.3f, 0.3f, 270.0f, 0.0f, 0.0f, 0.0f,    120, 10,   0.9f, 0.05f,   10,1,   0,0, 0,-1));
            break;
        case 11: // Headless bomberman - pouze Meme mody
            sDisplay->AddModelFeature(pRecord, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(65, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));
            break;
    }
}

/** \brief Update funkce nepritele
 *
 * Pokud neni mrtvy, dojde ke zvoleni generatoru cesty podle dispozic, pokud je nutne obnovit cestu (vyprseni casovace)
 *
 * Funkce ve svem tele obsahuje blizsi popis AI levelu
 */
void EnemyTemplate::Update()
{
    if (sGameplayMgr->IsSingleGameType() && sApplication->GetStagePhase() == 2)
        return;

    // Samotny update

    clock_t tnow = clock();

    if (!IsDead())
    {
        uint32 mx = ceil(pRecord->x);
        uint32 my = ceil(pRecord->z);

        // Pokud jsme se posunuli na jine pole ve 2D mape, musime osetrit, zdali jsme nestoupnuli na nebezpecne pole
        if (mx != m_enemyX || my != m_enemyY)
        {
            m_enemyX = mx;
            m_enemyY = my;

            if (sGameplayMgr->IsDangerousField(m_enemyX, m_enemyY))
            {
                SetDead(true);
                if (m_movement)
                    m_movement->Mutate(MOVEMENT_NONE);

                // TODO: osetrit tohle metodou herniho typu!
                if (sGameplayMgr->GetGameType() == GAME_TYPE_SP_CLASSIC)
                    sGameplayMgr->localPlayerStats.ClassicSingleStats.enemiesKilled += 1;
                else if (sGameplayMgr->GetGameType() == GAME_TYPE_SP_MEME)
                    sGameplayMgr->localPlayerStats.MemeSingleStats.enemiesTrolled += 1;

                return;
            }
        }

        if (m_movement)
        {
            // Pokud uz je cas na update typu pohybu
            if (m_nextMoveTypeUpdate <= tnow)
            {
                /*  Rozdeleni AI podle jeho urovne uvazovani
                 *
                 * level 1 = uplny blbec, jen se nahodne potlouka kolem
                 * level 2 = trochu chytrejsi, hleda si cestu ven z nebezpecnych poli
                 * level 3 = navic jeste hleda cestu k hraci pokud tim neohrozi sebe
                 *
                 *  Do budoucna:
                 * level 4 = je schopny polozit bombu
                 * level 5 = navic jeste spolupracuje s jinymi neprateli
                 */

                if (m_AILevel >= 3)
                {
                    // Nejdrive zkusime, zdali se nejak muzeme dostat k cili
                    if (m_movement->GetMovementType() != MOVEMENT_TARGETTED)
                    {
                        if (!m_movement->TryMutate(MOVEMENT_TARGETTED))
                            m_movement->TryMutate(MOVEMENT_OUTOFZERO);
                    }
                }
                if (m_AILevel >= 2)
                {
                    // Pokud cesta neexistuje, muze to byt i tim, ze stojime na zablokovanem miste bombou nebo plamenem
                    if (!m_movement->HasPath())
                        m_movement->TryMutate(MOVEMENT_OUTOFZERO);
                }
                if (m_AILevel >= 1)
                {
                    // Pokud stale cesta neexistuje, jdeme tedy nahodne
                    if ((m_movement->GetMovementType() == MOVEMENT_TARGETTED || m_movement->GetMovementType() == MOVEMENT_OUTOFZERO) && !m_movement->HasPath())
                    {
                        // 2 a vice se umi vyhybat nebezpecnym polim, nizsi do nich muzou vejit a "spachat sebevrazdu"
                        if (m_AILevel < 2)
                            m_movement->TryMutate(MOVEMENT_RANDOM_BARE);
                        else
                            m_movement->TryMutate(MOVEMENT_RANDOM);
                    }
                }

                m_nextMoveTypeUpdate = tnow + HOLDER_UPDATE_DELAY * m_movement->GetSpeedMod();
            }

            // A samotny update pohybu
            // Musi byt spusten pokazde - tam se posunuje model po vektoru
            m_movement->Update();
        }
    }
}
