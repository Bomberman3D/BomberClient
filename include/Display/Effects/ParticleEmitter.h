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

/** \var Particle::m_record
 *  \brief Vlastni zaznam modelu nebo billboardu v displaylistu tridy DisplayMgr
 */

/** \var Particle::m_timeStart
 *  \brief Cas, ve kterem byla castice vytvorena [ms]
 */

/** \var Particle::m_timeMax
 *  \brief Cas, ve kterem bude castice znicena [ms]
 */

/** \var Particle::m_startX
 *  \brief Pocatecni Xova souradnice
 */

/** \var Particle::m_startY
 *  \brief Pocatecni Yova souradnice
 */

/** \var Particle::m_startZ
 *  \brief Pocatecni Zova souradnice
 */

/** \var Particle::trajVector
 *  \brief Smerovy vektor castice (vektor jeji trajektorie)
 */

typedef std::list<Particle*> ParticleList;

/** \brief Struktura zaznamu metace castic
 */
struct Emitter
{
    // Staticka data, nastavi se pri vytvoreni
    EmitterType m_type;
    DisplayListRecord* m_template;
    float m_centerX, m_centerY, m_centerZ;
    float m_width, m_height;
    float m_angleMedX, m_angleMedY, m_angleMedZ;
    float m_angleTolX, m_angleTolY, m_angleTolZ;
    uint32 m_timeMed;
    uint32 m_timeTol;
    uint32 m_particleTimeMed;
    uint32 m_particleTimeTol;
    float m_speedMed;
    float m_speedTol;
    clock_t m_endTime;
    bool m_emitting;
    uint32 m_emitAnim;
    uint32 m_emitAnimFrameSkip;
    uint8  m_emitAnimFlags;

    CVector3 startVector[2];

    ParticleList m_Particles;
    clock_t m_nextParticleTime;

    bool Update();
};

/** \fn Emitter::Update()
 *  \brief Update funkce emitteru, stara se o posun castic, tvorbu novych a zahazovani starych
 *
 * Vraci false, pokud je nutne emitter zahodit (skoncila doba emittovani).
 * Je volana z ParticleEmitterMgr::Update
 */

/** \var Emitter::m_type
 *  \brief Typ emitteru (model / billboard)
 */

/** \var Emitter::m_template
 *  \brief Predloha pro castici ve forme zaznamu displaylistu (predloha ale neni vlozena do displaylistu, jen ma formu jeho zaznamu)
 */

/** \var Emitter::m_centerX
 *  \brief Xova souradnice stredu metani
 */

/** \var Emitter::m_centerY
 *  \brief Yova souradnice stredu metani
 */

/** \var Emitter::m_centerZ
 *  \brief Zova souradnice stredu metani
 */

/** \var Emitter::m_width
 *  \brief Sirka emittovani
 */

/** \var Emitter::m_height
 *  \brief Vyska emittovani
 */

/** \var Emitter::m_angleMedX
 *  \brief Stredni hodnota uhlu po ose (platne pro vsechny 3 promenne tohoto typu)
 */

/** \var Emitter::m_angleTolX
 *  \brief Povolena odchylka uhlu po ose (+/-) (platne pro vsechny 3 promenne tohoto typu)
 */

/** \var Emitter::m_timeMed
 *  \brief Stredni hodnota casu jedne castice [ms]
 */

/** \var Emitter::m_timeTol
 *  \brief Povolena odchylka casu jedne castice [ms]
 */

/** \var Emitter::m_particleTimeMed
 *  \brief Stredni hodnota casu mezi casticemi [ms]
 */

/** \var Emitter::m_particleTimeTol
 *  \brief Povolena odchylka casu mezi casticemi [ms]
 */

/** \var Emitter::m_speedMed
 *  \brief Stredni hodnota rychlosti za 1 sekundu [j/s]
 */

/** \var Emitter::m_speedTol
 *  \brief Povolena odchylka rychlosti za 1 sekundu [j/s]
 */

/** \var Emitter::m_endTime
 *  \brief Cas konce metani daneho emitteru [ms]
 */

/** \var Emitter::m_emitting
 *  \brief Priznak pro metani - pokud je false, netvori se dalsi castice
 */

/** \var Emitter::m_emitAnim
 *  \brief Animace emittovanych castic
 */

/** \var Emitter::m_emitAnimFrameSkip
 *  \brief Rychlost animace emittovanych castic
 */

/** \var Emitter::m_emitAnimFlags
 *  \brief Priznaky pro animaci emittovanych castic
 */

/** \var Emitter::startVector
 *  \brief Uhlopricne vektory obdelnika emittovani
 */

/** \var Emitter::m_Particles
 *  \brief List vsech castic emitteru
 */

/** \var Emitter::m_nextParticleTime
 *  \brief Cas na vypusteni dalsi castice [ms]
 */

typedef std::list<Emitter*> EmitterList;

class ParticleEmitterMgr
{
    public:
        ParticleEmitterMgr();
        void Update();

        Emitter* AddEmitter(DisplayListRecord* templ, float centerX, float centerY, float centerZ, float width, float height,
                            float angleMedX, float angleMedY, float angleTolX, float angleTolY,
                            uint32 timeMed, uint32 timeTol, float speedMed, float speedTol, uint32 particleTimeMed,
                            uint32 particleTimeTol, uint32 anim, uint32 animFrameSkip, uint8 animFlags, int32 duration);
        void RemoveEmitter(Emitter* which);

        void FlushEmitters();
        void PauseEmitters();
        void UnpauseEmitters();

    private:
        EmitterList Emitters;
        clock_t m_pauseTime;
};

#define sParticleEmitterMgr Singleton<ParticleEmitterMgr>::instance()

#endif
