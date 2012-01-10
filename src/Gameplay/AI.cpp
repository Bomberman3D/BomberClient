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
    // Pokud stojime na nepristupnem poli, jsme uvezneni
    if (accessMatrixDyn[std::make_pair(m_sourceX, m_sourceY)] == 0)
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
    if (accessMatrixDyn[std::make_pair(m_sourceX, m_sourceY)] == 0)
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
/*               Movement holder class                              */
/*                                                                  */
/********************************************************************/

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

MovementHolder::~MovementHolder()
{
}

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
    }
}

bool MovementHolder::TryGenerator(MovementType moveType)
{
    // TryGenerator slouzi k vygenerovani cesty k cili, ale do docasne vedlejsi mapy
    // pote je overeno funkci TryMutate, zdali doslo k vygenerovani a pripadne se pouzije
    // vygenerovana cesta
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
    }

    return false;
}

void MovementHolder::Mutate(MovementType moveType)
{
    // Funkce mutate slouzi k nastaveni generatoru cesty "na jistotu"
    // nebo k implicitnimu nastaveni

    if (moveType >= MOVEMENT_MAX)
        return;

    m_moveType = moveType;
    Generator();

    // Zmenime animaci modelu - je treba presunout asi, tady se mi to nelibi
    if (sAnimator->GetAnimId(m_src->pRecord->AnimTicket) != ANIM_WALK)
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 5);
}

bool MovementHolder::TryMutate(MovementType moveType)
{
    // TryMutate overi, zdali existuje cesta pomoci generatoru, ktery byl zvolen,
    // a pokud ano, pouzije se vygenerovana cesta a nastavi se typ pohybu

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
            sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 5);

        return true;
    }

    return false;
}

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
        sAnimator->ChangeModelAnim(m_src->pRecord->AnimTicket, ANIM_WALK, 0, 5);

    float timePass = 500 * m_speedMod;

    // Posun modelu po vektoru pohybu
    m_src->pRecord->x = m_path[m_currentPathNode].x - 0.5f + ( m_nodeVector.x*( float(tnow-m_nodeStartTime)/timePass ));
    m_src->pRecord->z = m_path[m_currentPathNode].y - 0.5f + ( m_nodeVector.y*( float(tnow-m_nodeStartTime)/timePass ));

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

bool MovementHolder::HasPath()
{
    return (m_path.size() > 1);
}

void EnemyTemplate::Init(uint32 modelId, uint32 x, uint32 y)
{
    // Inicializace modelu a zaznamu modelu pro nepritele
    pRecord = sDisplay->DrawModel(modelId, x-0.5f, 0.0f, y-0.5f, ANIM_IDLE, 0.45f, 0.0f, true);
}

void EnemyTemplate::Update()
{
    // TODO: podminka na singleplayer ?
    if (sApplication->GetStagePhase() == 2)
        return;

    // Samotny update

    clock_t tnow = clock();

    if (m_movement)
    {
        // Pokud uz je cas na update typu pohybu
        if (m_nextMoveTypeUpdate <= tnow)
        {
            // Nejdrive zkusime, zdali se nejak muzeme dostat k cili
            if (m_movement->GetMovementType() != MOVEMENT_TARGETTED)
                m_movement->TryMutate(MOVEMENT_TARGETTED);

            // Pokud ne (cesta neexistuje), presedlame na nahodny pohyb
            if (m_movement->GetMovementType() == MOVEMENT_TARGETTED && !m_movement->HasPath())
                m_movement->TryMutate(MOVEMENT_RANDOM);

            m_nextMoveTypeUpdate = tnow + HOLDER_UPDATE_DELAY * m_movement->GetSpeedMod();
        }

        // A samotny update pohybu
        // Musi byt spusten pokazde - tam se posunuje model po vektoru
        m_movement->Update();
    }
}
