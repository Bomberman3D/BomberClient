#ifndef BOMB_PARTICLE_EMITTER
#define BOMB_PARTICLE_EMITTER

#include <Global.h>
#include <Singleton.h>
#include <Display.h>

// Typ emitteru
enum EmitterType
{
    EMITTER_NONE      = 0,
    EMITTER_MODEL     = 1,
    EMITTER_BILLBOARD = 2,
    EMITTER_MAX,
};

// Jedna castice
struct Particle
{
    DisplayListRecord* m_record;
    clock_t m_timeStart; // Jak dlouho je jiz na scene [ms]
    clock_t m_timeMax;   // Cas smrti [ms]
    float m_startX, m_startY, m_startZ;
    CVector3 trajVector; // vektor pohybu (gravitace se nepocita, odecita se pak)
                         // velikost vektoru urcuje vzdalenost kterou castice urazi za 1 sekundu
};

typedef std::list<Particle*> ParticleList;

struct Emitter
{
    // Staticka data, nastavi se pri vytvoreni
    EmitterType m_type;
    DisplayListRecord* m_template;
    float m_centerX, m_centerY, m_centerZ;
    float m_width, m_height;
    float m_angleMedX, m_angleMedY, m_angleMedZ; // Stredni hodnota uhlu
    float m_angleTolX, m_angleTolY, m_angleTolZ; // +-uhel na kazde ose
    uint32 m_timeMed;           // Stredni hodnota casu [ms]
    uint32 m_timeTol;           // +-cas, ktery muze castice mit [ms]
    uint32 m_particleTimeMed;   // cas na dalsi castici [ms]
    uint32 m_particleTimeTol;   // +-cas na dalsi castici [ms]
    float m_speedMed;           // vzdalenost za 1 sekundu
    float m_speedTol;           // +-vzdalenost za 1 sekundu
    clock_t m_endTime;          // zivotnost metace
    bool m_emitting;            // aktivni? - nastavi se pri ukoncovani a dobihani castic
    uint32 m_emitAnim;          // animace castic
    uint32 m_emitAnimFrameSkip; // rychlost animace (skip framu) castic
    // Nastaveno pri vytvoreni
    CVector3 startVector[2];    // Vektor pouzity k randomizaci startovni pozice (uhlopricky v startovnim obdelniku)

    // Dynamicka data
    ParticleList m_Particles;   // List vsech castic tohohle emitteru
    clock_t m_nextParticleTime; // Cas dalsi castice [ms]

    // Funkce volaná z ParticleEmitterMgr::Update
    bool Update();
};

typedef std::list<Emitter*> EmitterList;

class ParticleEmitterMgr
{
    public:
        ParticleEmitterMgr();
        void Update();

        Emitter* AddEmitter(DisplayListRecord* templ, float centerX, float centerY, float centerZ, float width, float height,
                            float angleMedX, float angleMedY, float angleTolX, float angleTolY,
                            uint32 timeMed, uint32 timeTol, float speedMed, float speedTol, uint32 particleTimeMed,
                            uint32 particleTimeTol, uint32 anim, uint32 animFrameSkip, int32 duration);

    private:
        EmitterList Emitters;
};

#define sParticleEmitterMgr Singleton<ParticleEmitterMgr>::instance()

#endif
