#include <Loader.h>

class DummyEnemyScript: public EnemyScript
{
    struct DummyEnemyAI: public EnemyAI
    {
        DummyEnemyAI(): EnemyAI()
        {
        }

        //
    };

    EnemyAI* GetAI()
    {
        return new DummyEnemyAI();
    }
};

void LoadDummyEnemyScripts()
{
    EnemyScript* pScript = new DummyEnemyScript();
    sScriptMgr->RegisterScript(1, pScript);
    sScriptMgr->RegisterScript(2, pScript);
    sScriptMgr->RegisterScript(3, pScript);
    sScriptMgr->RegisterScript(4, pScript);
    sScriptMgr->RegisterScript(5, pScript);
}
