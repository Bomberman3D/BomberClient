#include <Global.h>
#include <ScriptMgr.h>
#include <Loader.h>

EnemyScript::EnemyScript()
{
    //
}

EnemyScript::~EnemyScript()
{
    //
}

ScriptMgr::ScriptMgr()
{
    //
}

ScriptMgr::~ScriptMgr()
{
    //
}

void ScriptMgr::Initialize()
{
    LoadScripts();
}

void ScriptMgr::RegisterScript(uint32 enemyId, EnemyScript *scr)
{
    if (m_enemyScriptMap.find(enemyId) != m_enemyScriptMap.end())
        fprintf(stdout, "Overriding enemyId script for ID %u\n", enemyId);

    m_enemyScriptMap[enemyId] = scr;
}

bool ScriptMgr::HasScript(uint32 enemyId)
{
    return (m_enemyScriptMap.find(enemyId) != m_enemyScriptMap.end());
}

EnemyAI* ScriptMgr::CreateEnemyAI(uint32 enemyId, EnemyTemplate* parent)
{
    EnemyAI* ret = NULL;

    // Pokud neexistuje custom, vytvorime prazdne dummy AI
    if (m_enemyScriptMap.find(enemyId) == m_enemyScriptMap.end())
    {
        ret = new EnemyAI();
        ret->SetParentScript(NULL);
    }
    else
    {
        ret = m_enemyScriptMap[enemyId]->GetAI();
        ret->SetParentScript(m_enemyScriptMap[enemyId]);
    }

    ret->SetParentEnemy(parent);

    return ret;
}
