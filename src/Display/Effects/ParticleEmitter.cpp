#include <Global.h>
#include <Helpers.h>
#include <Display.h>
#include <Effects/ParticleEmitter.h>
#include <Effects/Animations.h>

ParticleEmitterMgr::ParticleEmitterMgr()
{
    Emitters.clear();
}

void ParticleEmitterMgr::Update()
{
    if (m_pauseTime > 0)
        return;

    // Pokud nemame zadny emitter, nemusime updatovat
    if (Emitters.empty())
        return;

    // Prochazi vsechny emittery
    for (EmitterList::iterator itr = Emitters.begin(); itr != Emitters.end();)
    {
        // A pokud update vrati false, vymazeme ho. Pokud ne, pokracujeme
        if ((*itr)->Update())
            ++itr;
        else
        {
            if ((*itr)->m_template)
                delete (*itr)->m_template;
            itr = Emitters.erase(itr);
        }
    }
}

void ParticleEmitterMgr::PauseEmitters()
{
    m_pauseTime = clock();
}

void ParticleEmitterMgr::UnpauseEmitters()
{
    clock_t diff = clock() - m_pauseTime;

    m_pauseTime = 0;

    for (EmitterList::iterator itr = Emitters.begin(); itr != Emitters.end(); ++itr)
        (*itr)->m_endTime += diff;
}

Emitter* ParticleEmitterMgr::AddEmitter(DisplayListRecord* templ, float centerX, float centerY, float centerZ, float width, float height,
                                    float angleMedX, float angleMedY, float angleTolX, float angleTolY,
                                    uint32 timeMed, uint32 timeTol, float speedMed, float speedTol,
                                    uint32 particleTimeMed, uint32 particleTimeTol, uint32 anim, uint32 animFrameSkip, int32 duration)
{
    if (!templ || templ->m_type == DL_TYPE_NONE)
        return NULL;

    Emitter* pTemp = new Emitter;

    pTemp->m_template  = templ;
    pTemp->m_centerX   = centerX;
    pTemp->m_centerY   = centerY;
    pTemp->m_centerZ   = centerZ;
    pTemp->m_width     = width;
    pTemp->m_height    = height;
    pTemp->m_angleMedX = angleMedX * PI / 180.0f;
    pTemp->m_angleMedY = angleMedY * PI / 180.0f;
    pTemp->m_angleTolX = angleTolX * PI / 180.0f;
    pTemp->m_angleTolY = angleTolY * PI / 180.0f;
    pTemp->m_timeMed   = timeMed;
    pTemp->m_timeTol   = timeTol;
    pTemp->m_particleTimeMed   = particleTimeMed;
    pTemp->m_particleTimeTol   = particleTimeTol;
    pTemp->m_speedMed  = speedMed;
    pTemp->m_speedTol  = speedTol;
    pTemp->m_endTime   = clock()+duration;
    pTemp->m_emitting  = true;
    pTemp->m_emitAnim  = anim;
    pTemp->m_emitAnimFrameSkip = animFrameSkip;

    // Uhly angleX a angleY (angleMed+angleTol) budou prohozene, kvuli lepsi predstavivosti
    // angleX bude tedy uhel otoceni po ose Y, ale defakto "sirka" emitovani - proto osa X
    // to same u angleY, to je defakto "vyska" emitovani, cili proto osa Y

    // Vypocet uhlopricek v obdelniku emittovani
    pTemp->startVector[0].x = pTemp->m_width / 2;
    pTemp->startVector[0].y = pTemp->m_height / 2;
    pTemp->startVector[0].z = 0;
    pTemp->startVector[1].x = -pTemp->m_width / 2;
    pTemp->startVector[1].y = pTemp->m_height / 2;
    pTemp->startVector[1].z = 0;

    // Transformace jednotlivych uhlopricnych vektoru je porad stejna, rozdilny je jen vstup na zacatku
    for (uint8 i = 0; i < 2; i++)
    {
        pTemp->startVector[i].z = - pTemp->startVector[i].y * sin(pTemp->m_angleMedX);
        pTemp->startVector[i].y = pTemp->startVector[i].y * cos(pTemp->m_angleMedX);

        pTemp->startVector[i].z = pTemp->startVector[i].z + pTemp->startVector[i].x * sin(pTemp->m_angleMedY);
        pTemp->startVector[i].x = pTemp->startVector[i].x * cos(pTemp->m_angleMedY);
    }

    pTemp->m_nextParticleTime = 0;
    pTemp->m_Particles.clear();

    Emitters.push_back(pTemp);

    return pTemp;
}

void ParticleEmitterMgr::RemoveEmitter(Emitter* which)
{
    which->m_endTime = clock();
}

void ParticleEmitterMgr::FlushEmitters()
{
    if (Emitters.empty())
        return;

    // Prochazi vsechny emittery
    for (EmitterList::iterator itr = Emitters.begin(); itr != Emitters.end();)
    {
        if ((*itr)->m_template)
            delete (*itr)->m_template;
        delete (*itr);
        itr = Emitters.erase(itr);
    }
}

bool Emitter::Update()
{
    clock_t tnow = clock();

    if (m_endTime <= tnow)
    {
        if (m_emitting)
            m_emitting = false;
        else
        {
            if (m_Particles.size() == 0)
                return false;
        }
    }

    if (m_emitting)
    {
        if (m_nextParticleTime <= tnow)
            for (int cnt = 0; cnt <= ((tnow-m_nextParticleTime)/(m_nextParticleTime?m_nextParticleTime:1)); cnt++)
            {
                Particle* pNew = new Particle;
                pNew->m_timeStart = tnow;

                // Castice bude zit timeMed casu a nahodne + nebo - z celkove tolerance
                // pokud timeTol == 0, tak samozrejme je cas konstantni
                if (m_timeTol == 0)
                    pNew->m_timeMax = tnow + m_timeMed;
                else
                    pNew->m_timeMax = tnow + m_timeMed + (rand()%(m_timeTol*2) - m_timeTol);

                // Vypocitame nasobky uhlopricnych vektoru
                /* Nasobky jednotlivych vektoru se nejdrive sectou, abychom dostali bod na obdelniku, ze ktereho
                * budou castice vychazet, a nasledne se vektorove vynasobi, abychom dostali vektor kolmy na oba
                * uhlopricne, cimz ziskame vektor trajektorie
                * Delky nasobku vektoru se mohou rovnat dohromady pouze polovine delky jedne uhlopricky (pripadne minus polovine)
                * protoze jedine tak dostaneme bod uvnitr obdelnika. Kdyby byly vetsi, dostali bychom se "ven"
                */
                float mult = frand(-1.0f,1.0f);
                float othermult = frand(fabs(mult)-1.0f,1.0f-fabs(mult));

                pNew->m_startX = m_centerX + mult*startVector[0].x + othermult*startVector[1].x;
                pNew->m_startY = m_centerY + mult*startVector[0].y + othermult*startVector[1].y;
                pNew->m_startZ = m_centerZ + mult*startVector[0].z + othermult*startVector[1].z;

                // vzdycky je v pocatku kolmy na obdelnik emittovani
                // pote se zapocita i nejaky nahodny uhel z rozmezi (angleMedX, angleMedY)
                pNew->trajVector = startVector[0].vectorMultiply(startVector[1]);

                // Prevede na jednotkovy vektor a vynasobi ho nasi pozadovanou "rychlosti", cili
                // vzdalenosti za 1 sekundu
                pNew->trajVector.unitMultiply(m_speedMed + frand(-m_speedTol, m_speedTol));

                // Nahodne pootocit okolo trech vektoru os (x,y a z)
                if (m_angleTolX != 0.0f)
                    pNew->trajVector.rotate(CVector3(1.0f, 0.0f, 0.0f), frand(-m_angleTolX, m_angleTolX)*PI/180.0f);
                if (m_angleTolY != 0.0f)
                    pNew->trajVector.rotate(CVector3(0.0f, 1.0f, 0.0f), frand(-m_angleTolY, m_angleTolY)*PI/180.0f);
                if (m_angleTolZ != 0.0f)
                    pNew->trajVector.rotate(CVector3(0.0f, 0.0f, 1.0f), frand(-m_angleTolZ, m_angleTolZ)*PI/180.0f);

                if (m_template->m_type == DL_TYPE_BILLBOARD)
                {
                    BillboardDisplayListRecord* temp = (BillboardDisplayListRecord*)m_template;
                    pNew->m_record = sDisplay->DrawBillboard(temp->textureId, temp->x, temp->y, temp->z, m_emitAnim, m_emitAnimFrameSkip, temp->scale_x, temp->scale_y,
                                                             temp->billboard_x, temp->billboard_y, true);
                }
                else if (m_template->m_type == DL_TYPE_MODEL)
                {
                    ModelDisplayListRecord* temp = (ModelDisplayListRecord*)m_template;
                    pNew->m_record = sDisplay->DrawModel(temp->modelId, temp->x, temp->y, temp->z, (ModelAnimType)m_emitAnim, temp->scale, temp->rotate, true, false, 0, 0, ANIM_RESTRICTION_NOT_PAUSED);
                }
                // hypoteticky jina moznost nastat nemuze

                m_Particles.push_front(pNew);

                m_nextParticleTime = tnow + m_particleTimeMed + (rand()%((m_particleTimeTol?m_particleTimeTol*2:1)) - m_particleTimeTol);
            }
    }

    Particle* p = NULL;
    for (ParticleList::iterator itr = m_Particles.begin(); itr != m_Particles.end(); )
    {
        p = (*itr);
        // Pokud castice neexistuje, nebo neexistuje displaylist zaznam nebo uz je na scene moc dlouho
        if (!p || !p->m_record)
        {
            if (p)
                delete p;
            itr = m_Particles.erase(itr);
            continue;
        }

        // Pokud uz castice dosahla maximalniho mozneho casu "na scene", vymazeme ji
        if (p->m_timeMax <= tnow)
        {
            p->m_record->remove = true;
            itr = m_Particles.erase(itr);
            continue;
        }

        // Posun castice po vektoru trajektorie
        // Pokud v budoucnu pribude i gravitace, bude se odecitat od celkove hodnoty Y
        p->m_record->x = p->m_startX + p->trajVector.x * ((tnow - p->m_timeStart)/1000.0f);
        p->m_record->y = p->m_startY + p->trajVector.y * ((tnow - p->m_timeStart)/1000.0f);
        p->m_record->z = p->m_startZ + p->trajVector.z * ((tnow - p->m_timeStart)/1000.0f);

        ++itr;
    }

    return true;
}
