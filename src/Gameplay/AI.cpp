#include <Global.h>
#include <AI.h>
#include <Map.h>

Pathfinder::Pathfinder()
{
    m_sourceX = 0;
    m_sourceY = 0;
    m_destX = 0;
    m_destY = 0;

    m_path.clear();
}

Pathfinder::~Pathfinder()
{
}

void Pathfinder::Initialize(uint32 sourceX, uint32 sourceY, uint32 destX, uint32 destY)
{
    m_sourceX = sourceX;
    m_sourceY = sourceY;
    m_destX = destX;
    m_destY = destY;

    m_path.clear();

    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap)
        return;

    m_mapSizeX = pMap->field.size();
    m_mapSizeY = pMap->field[0].size();

    accessMatrixOrigin.clear();
    accessMatrixDyn.clear();
    CoordPair tmp;
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
                // Vynulujeme ho v hledacich mapach
                accessMatrixOrigin[tmp] = 0;
                accessMatrixDyn[tmp] = 0;
                continue;
            }

            // Pokud je pristupne, jednicku
            accessMatrixOrigin[tmp] = 1;
            accessMatrixDyn[tmp] = 1;
        }
    }

    // Vynulovat startovni pozici - tam ted jsme
    //tmp = std::make_pair(sourceX, sourceY);
    //accessMatrixOrigin[tmp] = 0;
    //accessMatrixDyn[tmp] = 0;
}

void Pathfinder::PrintAndWait()
{
    fprintf(stdout, "------------\n");
    for (uint32 i = 0; i < m_mapSizeX; i++)
    {
        for (uint32 j = 0; j < m_mapSizeY; j++)
        {
            fprintf(stdout, "%i", accessMatrixDyn[std::make_pair(i,j)]);
        }
        fprintf(stdout, "\n");
    }
    system("pause");
}

#define RECURSE_RIGHT   if ((x+1) < m_mapSizeX && accessMatrixDyn[std::make_pair(x+1,y)] == 1) \
                        {                         \
                            if (Recursor(x+1, y)) \
                            {                     \
                                p->x = x+1;       \
                                p->y = y;         \
                                p->flags = 0;     \
                                accessMatrixDyn[std::make_pair(x+1,y)] = 9; \
                                m_path.push_back(*p); \
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
                                m_path.push_back(*p); \
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
                                m_path.push_back(*p); \
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
                                m_path.push_back(*p); \
                                return true;      \
                            }                     \
                        }

bool Pathfinder::Recursor(uint32 x, uint32 y)
{
    // pozdeji zmenit na == 0, ted jen pro nazornost se budou nastavovat dvojky jako prosla cesta
    if (accessMatrixDyn[std::make_pair(x,y)] != 1)
        return false;

    accessMatrixDyn[std::make_pair(x,y)] = 2;

    if (x == m_destX && y == m_destY)
        return true;

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
    }
}
