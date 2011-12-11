#include <Global.h>
#include <Gameplay.h>

GameplayMgr::GameplayMgr()
{
    m_game = NULL;
}

GameplayMgr::~GameplayMgr()
{
    if (m_game)
        delete m_game;
}

void GameplayMgr::Update()
{
    if (m_game)
        m_game->OnUpdate();
}

void GameplayMgr::OnGameInit()
{
    if (m_game)
        m_game->OnGameInit();
}

void GameplayMgr::SetGameType(GameType type)
{
    if (type == GAME_TYPE_MAX)
        return;

    if (m_game)
        delete m_game;

    switch (type)
    {
        case GAME_TYPE_NONE:
            m_game = new GameTypeTemplate;
            break;
        case GAME_TYPE_SP_CLASSIC:
            m_game = new ClassicSingleGameType;
            break;
        case GAME_TYPE_MP_CLASSIC:
            m_game = new ClassicMultiGameType;
            break;
        default:
            break;
    }

    assert(m_game != NULL);
}

GameType GameplayMgr::GetGameType()
{
    if (!m_game)
        return GAME_TYPE_NONE;

    return m_game->GetType();
}
