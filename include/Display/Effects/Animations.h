#ifndef BOMB_ANIMATIONS_H
#define BOMB_ANIMATIONS_H

#include <Global.h>
#include <Singleton.h>
#include <Storage.h>
#include <ctime>

enum AnimRecordType
{
    ANIM_TYPE_UNKNOWN = 0,
    ANIM_TYPE_TEXTURE = 1,
    ANIM_TYPE_MODEL   = 2,
};

enum AnimFlags
{
    ANIM_FLAG_BOTHWAY            = 0x01,
    ANIM_FLAG_FORCE_LOADING      = 0x02,
    ANIM_FLAG_NOT_REPEAT         = 0x04,
};

struct AnimationRecord
{
    // Staticka data
    uint32 sourceId;
    uint32 animId;
    uint8  animType;
    uint32 frameSkipSpeed;
    bool   bothWay; // dovoluje animovat nejdriv k nejvyssimu a pak k nejnizsimu framu
    uint8  animFlags;

    // Dynamicka data
    uint32 nextFrameTime;
    uint32 actualTextureId; // Kvuli rychlosti se bude uchovavat oboji
    uint32 actualFrame;
    bool   reversed;
    bool   disabled;
};

/** \var AnimationRecord::sourceId
 *  \brief Urcuje ID zdroje (textury nebo modelu)
 */

/** \var AnimationRecord::animID
 *  \brief ID animace (viz SQLite databaze a tabulky *_animation)
 */

/** \var AnimationRecord::animType
 *  \brief Typ zdroje (textura nebo model)
 */

/** \var AnimationRecord::frameSkipSpeed
 *  \brief Urcuje, kolik se ma preskocit snimku, kdyz nastane cas na animaci
 */

/** \var AnimationRecord::bothWay
 *  \brief Urcuje, zdali se po dosazeni maximalniho snimku ma zmenit smer animace (od nejvyssiho po nejnizsi snimek)
 */

/** \var AnimationRecord::animFlags
 *  \brief Priznaky animace - viz. enumerator AnimFlags
 */

/** \var AnimationRecord::nextFrameTime
 *  \brief Dynamicka promenna urcujici cas, ve kterem se ma prepnout snimek na dalsi
 */

/** \var AnimationRecord::actualTextureId
 *  \brief Uklada aktualni texturu (platne pouze pro animace textur)
 */

/** \var AnimationRecord::actualFrame
 *  \brief Uklada aktualni frame animace
 */

/** \var AnimationRecord::reversed
 *  \brief Momentalni stav toho, zdali se snimky inkrementuji nebo dekrementuji (podle bothWay parametru)
 */

/** \var AnimationRecord::disabled
 *  \brief Promenna urcujici, zdali animace bezi nebo je zastavena
 */

class Animator
{
    public:
        Animator();

        // System vyzvedavani, vyhledavani a znaceni ticketu
        uint32 GetTextureAnimTicket(uint32 textureId, uint32 animId, uint32 frameSkipSpeed = 1, uint8 flags = 0);
        void DestroyAnimTicket(uint32 id);
        uint32 GetModelAnimTicket(uint32 modelId, uint32 animId, uint32 frameSkipSpeed = 1, uint8 flags = 0);
        void ChangeModelAnim(uint32 ticketId, uint32 animId, uint32 startFrame = 0, uint32 frameSkipSpeed = 0, uint8 flags = 0);
        uint32 GetActualTexture(uint32 id);
        uint32 GetActualFrame(uint32 id);
        uint32 GetAnimId(uint32 id);
        void EnableAnimation(uint32 id);
        void DisableAnimation(uint32 id);

        void Update();

    private:
        // Interni funkce
        uint32 GetLowestAnimTicketId();

        // Typedefs
        typedef std::map<uint32, AnimationRecord> AnimMap;

        // Uloziste dynamickych dat
        AnimMap Anims;

        // Update veci
        clock_t m_lastUpdate;
        uint32 m_diff;
};

#define sAnimator Singleton<Animator>::instance()

class CustomAnimator
{
    public:
        CustomAnimator();

        bool HaveModelCustomAnim(uint32 id);
        void AnimateModelObjectByFrame(t3DObject *object, uint32 modelId, uint32 frame);
        void AnimateModelObjectByFrame(t3DObject *object, t3DModel* model, uint32 modelId, uint32 frame);
        void AnimateModelObjectByFrame(t3DObject *object, t3DModel* model, uint32 modelId, uint32 frame, ModelAnimType anim);

        void AnimateModelFeatures(ModelDisplayListRecord* record);
};

#define sCustomAnimator Singleton<CustomAnimator>::instance()

#endif
