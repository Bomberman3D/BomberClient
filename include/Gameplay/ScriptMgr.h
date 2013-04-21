#ifndef BOMB_SCRIPTMGR_H
#define BOMB_SCRIPTMGR_H

#include <Global.h>
#include <Singleton.h>
#include <AI.h>

class EnemyScript;

struct EnemyAI
{
    public:
        EnemyAI() {};
        ~EnemyAI() {};

        void SetParentScript(EnemyScript* scrparent) { parent = scrparent; };
        void SetParentEnemy(EnemyTemplate* enparent) { me = enparent; };

        virtual uint32 OverrideModelId(uint32 modelId) { return modelId; }
        virtual void OnCreate() {};

        virtual void OnUpdate() {};
        virtual bool OnMovementUpdate() { return false; };
        virtual void OnDead() {};
        virtual void OnDamageHit() {};
        virtual bool AvoidDeath() { return false; };
        virtual void OnFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY) {};
        virtual void OnMovementGeneratorChange(uint32 oldMovement, uint32 newMovement) {};

    protected:
        EnemyScript* parent;
        EnemyTemplate* me;
};

class EnemyScript
{
    public:
        EnemyScript();
        ~EnemyScript();

        virtual EnemyAI* GetAI() { return NULL; };

    protected:
};

class ScriptMgr
{
    public:
        ScriptMgr();
        ~ScriptMgr();

        void Initialize();

        void RegisterScript(uint32 enemyId, EnemyScript* scr);
        bool HasScript(uint32 enemyId);
        EnemyAI* CreateEnemyAI(uint32 enemyId, EnemyTemplate* parent);

    private:
        std::map<uint32, EnemyScript*> m_enemyScriptMap;
};

#define sScriptMgr Singleton<ScriptMgr>::instance()

#endif
