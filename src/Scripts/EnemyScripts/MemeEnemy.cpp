#include <Loader.h>
#include <Display.h>
#include <Gameplay.h>

class MemeCommonEnemyScript: public EnemyScript
{
    struct MemeCommonEnemyAI: public EnemyAI
    {
        MemeCommonEnemyAI(): EnemyAI()
        {
        }

        void OnCreate()
        {
            sDisplay->AddModelFeature(me->pRecord, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(65, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));
        }

        bool OnMovementUpdate()
        {
            return false;
        }

        void OnMovementGeneratorChange(uint32 /*oldm*/, uint32 newm)
        {
            //
        }
    };

    EnemyAI* GetAI()
    {
        return new MemeCommonEnemyAI();
    }
};

class MemeTrollEnemyScript: public EnemyScript
{
    struct MemeTrollEnemyAI: public EnemyAI
    {
        MemeTrollEnemyAI(): EnemyAI()
        {
        }

        clock_t nextFlameThrow;
        clock_t nextMoveStart;

        void OnCreate()
        {
            sDisplay->AddModelFeature(me->pRecord, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(62, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));
            nextFlameThrow = clock() + 3000;
            nextMoveStart = 0;
        }

        void OnUpdate()
        {
            if (nextFlameThrow <= clock())
            {
                me->m_movement->Mutate(MOVEMENT_NONE);
                nextFlameThrow = clock() + 3000;
                nextMoveStart = clock() + 1000;

                BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(31, 0, 0, 0, 0.5f, 0.5f, true, true);
                Emitter* pEmit = sParticleEmitterMgr->AddPointEmitter(templ, me->pRecord->x-0.5f, 0.1f, me->pRecord->z-0.5f, 0, me->pRecord->rotate-90.0f, 0, 0, 100, 20, 10.0f, 0.1f, 30, 8, 1, 0, ANIM_FLAG_NOT_REPEAT, 1000);
                sDisplay->AddModelFeature(me->pRecord, MF_TYPE_EMITTER, 0.0f, 1.55f, 0.0f, pEmit);
            }

            if (nextMoveStart)
            {
                if (nextMoveStart <= clock())
                {
                    nextMoveStart = 0;
                    me->m_movement->TryMutate(MOVEMENT_RANDOM);
                }
            }
        }

        bool OnMovementUpdate()
        {
            return false;
        }

        void OnMovementGeneratorChange(uint32 /*oldm*/, uint32 newm)
        {
            //
        }
    };

    EnemyAI* GetAI()
    {
        return new MemeTrollEnemyAI();
    }
};

class MemeRageEnemyScript: public EnemyScript
{
    struct MemeRageEnemyAI: public EnemyAI
    {
        MemeRageEnemyAI(): EnemyAI()
        {
        }

        clock_t nextEmitting;
        clock_t nextDestructing;
        clock_t nextFaceReturn;

        void OnCreate()
        {
            sDisplay->AddModelFeature(me->pRecord, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(85, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));
            nextEmitting = clock() + 3000;
            nextDestructing = 0;
            nextFaceReturn = 0;
        }

        void OnUpdate()
        {
            if (nextEmitting <= clock())
            {
                sDisplay->ClearModelFeaturesByType(me->pRecord, MF_TYPE_BILLBOARD);
                sDisplay->AddModelFeature(me->pRecord, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(81, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));

                me->m_movement->Mutate(MOVEMENT_NONE);
                nextEmitting = clock() + 15000;
                nextDestructing = clock() + 1000;

                BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(83, 0, 0, 0, 0.3f, 0.3f, true, true);
                Emitter* pEmit = sParticleEmitterMgr->AddPointEmitter(templ, me->pRecord->x-0.5f, 0.1f, me->pRecord->z-0.5f, 90.0f, 0, 90.0f, 180.0f, 100, 20, 10.0f, 0.1f, 100, 10, 0, 0, 0, 1000);
                sDisplay->AddModelFeature(me->pRecord, MF_TYPE_EMITTER, 0.0f, 2.05f, 0.0f, pEmit);
            }

            if (nextDestructing)
            {
                if (nextDestructing <= clock())
                {
                    nextDestructing = 0;
                    nextFaceReturn = clock() + 2500;

                    sDisplay->ClearModelFeaturesByType(me->pRecord, MF_TYPE_BILLBOARD);
                    sDisplay->AddModelFeature(me->pRecord, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(82, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));

                    me->m_movement->TryMutate(MOVEMENT_RANDOM);
                    BillboardDisplayListRecord* templ = BillboardDisplayListRecord::Create(84, 0, 0, 0, 0.3f, 0.3f, true, true);
                    Emitter* pEmit = sParticleEmitterMgr->AddPointEmitter(templ, me->pRecord->x-0.5f, 0.1f, me->pRecord->z-0.5f, 90.0f, 0, 90.0f, 180.0f, 100, 20, 15.0f, 0.5f, 30, 8, 0, 0, 0, 2500);
                    sDisplay->AddModelFeature(me->pRecord, MF_TYPE_EMITTER, 0.0f, 2.05f, 0.0f, pEmit);
                    sDisplay->AddCameraShakePoint(me->pRecord, 5.0f, 15.0f, 2500);
                }
            }

            if (nextFaceReturn)
            {
                if (nextFaceReturn <= clock())
                {
                    nextFaceReturn = 0;

                    sDisplay->ClearModelFeaturesByType(me->pRecord, MF_TYPE_BILLBOARD);
                    sDisplay->AddModelFeature(me->pRecord, MF_TYPE_BILLBOARD, 0.0f, 1.55f, 0.0f, sDisplay->DrawBillboard(85, 0, 0, 0, 0, 1, 0.4f, 0.45f, false, true));
                }
            }
        }

        bool OnMovementUpdate()
        {
            return false;
        }

        void OnMovementGeneratorChange(uint32 /*oldm*/, uint32 newm)
        {
            //
        }
    };

    EnemyAI* GetAI()
    {
        return new MemeRageEnemyAI();
    }
};

void LoadMemeEnemyScripts()
{
    EnemyScript* pScript = new MemeCommonEnemyScript();
    sScriptMgr->RegisterScript(6, pScript);
    sScriptMgr->RegisterScript(7, pScript);

    pScript = new MemeTrollEnemyScript();
    sScriptMgr->RegisterScript(8, pScript);

    pScript = new MemeRageEnemyScript();
    sScriptMgr->RegisterScript(9, pScript);
}
