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
    CoordPair tmp;

    // "Pristupova mapa" se naplni jednickami tam, kde pathfinder muze hledat cestu
    // a nulami tam, kam nemuze
    for (uint32 i = 0; i < m_mapSizeX; i++)
    {
        for (uint32 j = 0; j < m_mapSizeY; j++)
        {
            tmp = std::make_pair(i,j);

            // Pokud je nepristupne pole na souradnicich [x,y]
            if (pMap->field[i][j].type == TYPE_SOLID_BOX
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOX)
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOMB))
            {
                // Vynulujeme ho v hledaci mape
                accessMatrixDyn[tmp] = 0;
                continue;
            }

            // Pokud je pristupne, jednicku
            accessMatrixDyn[tmp] = 1;
        }
    }
}

// Makra pro rekurzivni postup po poli
// ve zkratce jen volaji dalsi zanoreni funkce Recursor a pri uspechu (true) zapisou bod do cesty
// Nastaveni na 9 je zde jen kvuli pripadnemu debugu a vystupu
#define RECURSE_RIGHT   if ((x+1) < m_mapSizeX && accessMatrixDyn[std::make_pair(x+1,y)] == 1) \
                        {                         \
                            if (Recursor(x+1, y)) \
                            {                     \
                                p->x = x+1;       \
                                p->y = y;         \
                                p->flags = 0;     \
                                accessMatrixDyn[std::make_pair(x+1,y)] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

#define RECURSE_DOWN    if ((y+1) < m_mapSizeY && accessMatrixDyn[std::make_pair(x,y+1)] == 1) \
                        {                         \
                            if (Recursor(x, y+1)) \
                            {                     \
                                p->x = x;         \
                                p->y = y+1;       \
                                p->flags = 0;     \
                                accessMatrixDyn[std::make_pair(x,y+1)] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

#define RECURSE_LEFT    if (x > 0 && accessMatrixDyn[std::make_pair(x-1,y)] == 1) \
                        {                         \
                            if (Recursor(x-1, y)) \
                            {                     \
                                p->x = x-1;       \
                                p->y = y;         \
                                p->flags = 0;     \
                                accessMatrixDyn[std::make_pair(x-1,y)] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

#define RECURSE_UP      if (y > 0 && accessMatrixDyn[std::make_pair(x,y-1)] == 1) \
                        {                         \
                            if (Recursor(x, y-1)) \
                            {                     \
                                p->x = x;         \
                                p->y = y-1;       \
                                p->flags = 0;     \
                                accessMatrixDyn[std::make_pair(x,y-1)] = 9; \
                                m_path->push_back(*p); \
                                return true;      \
                            }                     \
                        }

bool Pathfinder::Recursor(uint32 x, uint32 y)
{
    // pozdeji zmenit na == 0, ted jen pro nazornost se budou nastavovat dvojky jako prosla cesta
    if (accessMatrixDyn[std::make_pair(x,y)] != 1)
        return false;

    // Automaticky nastavime pole jako "navstivene"
    accessMatrixDyn[std::make_pair(x,y)] = 2;

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
    if (!Recursor(m_sourceX, m_sourceY))
    {
        // Cesta se nevygenerovala, nutno nasadit nahodny pohyb
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
    CoordPair tmp;

    // "Pristupova mapa" se naplni jednickami tam, kde pathfinder muze hledat cestu
    // a nulami tam, kam nemuze
    for (uint32 i = 0; i < m_mapSizeX; i++)
    {
        for (uint32 j = 0; j < m_mapSizeY; j++)
        {
            tmp = std::make_pair(i,j);

            // Pokud je nepristupne pole na souradnicich [x,y]
            if (pMap->field[i][j].type == TYPE_SOLID_BOX
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOX)
                || pMap->IsDynamicRecordPresent(i,j,DYNAMIC_TYPE_BOMB))
            {
                // Vynulujeme ho v hledaci mape
                accessMatrixDyn[tmp] = 0;
                continue;
            }

            // Pokud je pristupne, jednicku
            accessMatrixDyn[tmp] = 1;
        }
    }
}

bool RandomPathfinder::Recursor(uint32 x, uint32 y)
{
    if (m_curLength >= m_length)
        return true;

    m_curLength++;

    PathNode* p = new PathNode;

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
/*               Movement holder class                              */
/*                                                                  */
/********************************************************************/

MovementHolder::MovementHolder(EnemyTemplate* src)
{
    m_path.clear();
    m_src = src;
    m_moveType = MOVEMENT_NONE;
    m_lastMoveUpdate = clock();
    m_nodeStartTime = 0;
}

MovementHolder::~MovementHolder()
{
}

void MovementHolder::Generator()
{
    switch (m_moveType)
    {
        case MOVEMENT_TARGETTED:
        {
            ModelDisplayListRecord* pTarget = sDisplay->GetTargetModel();
            if (!pTarget)
            {
                m_moveType = MOVEMENT_NONE;
                break;
            }

            uint32 mx = ceil(m_src->pRecord->x + 0.5f);
            uint32 my = ceil(m_src->pRecord->z + 0.5f);
            uint32 tx = ceil(pTarget->x);
            uint32 ty = ceil(pTarget->z);
            Pathfinder p(&m_path);
            p.Initialize(mx, my, tx, ty);
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
            uint32 mx = ceil(m_src->pRecord->x + 0.5f);
            uint32 my = ceil(m_src->pRecord->z + 0.5f);
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
    }
}

void MovementHolder::MutateToTargetGen()
{
    ModelDisplayListRecord* pTarget = sDisplay->GetTargetModel();
    if (!pTarget)
    {
        m_path.clear();
        return;
    }

    m_moveType = MOVEMENT_TARGETTED;
    Generator();

    if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_WALK)
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 5);
}

void MovementHolder::MutateToRandomGen()
{
    m_moveType = MOVEMENT_RANDOM;
    Generator();

    if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_WALK)
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 5);
}

void MovementHolder::Update()
{
    clock_t tnow = clock();

    if (m_path.size() < 2)
    {
        if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_IDLE)
            sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_IDLE, 0, 5);

        if (tnow < m_nextUpdate)
            return;

        Generator();

        m_nextUpdate = tnow + HOLDER_UPDATE_DELAY;
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

    if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_WALK)
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 5);

    m_src->pRecord->x = m_path[m_currentPathNode].x - 0.5f + ( m_nodeVector.x*( float(tnow-m_nodeStartTime)/500 ));
    m_src->pRecord->z = m_path[m_currentPathNode].y - 0.5f + ( m_nodeVector.y*( float(tnow-m_nodeStartTime)/500 ));

    if (tnow-m_nodeStartTime >= 500)
    {
        m_currentPathNode++;
        if (m_currentPathNode >= m_path.size()-1)
        {
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

void EnemyTemplate::Init(uint32 modelId, uint32 x, uint32 y)
{
    pRecord = sDisplay->DrawModel(modelId, x-0.5f, 0.0f, y-0.5f, ANIM_IDLE, 0.45f, 0.0f, true);
}

void EnemyTemplate::Update()
{
    if (m_movement)
        m_movement->Update();
}
