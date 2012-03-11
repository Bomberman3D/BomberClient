#ifndef BOMB_ANIMATIONS_H
#define BOMB_ANIMATIONS_H

#include <Global.h>
#include <Singleton.h>
#include <ctime>

enum AnimRecordType
{
    ANIM_TYPE_UNKNOWN = 0,
    ANIM_TYPE_TEXTURE = 1,
    ANIM_TYPE_MODEL   = 2,
};

struct AnimationRecord
{
    // Staticka data
    uint32 sourceId;
    uint32 animId;
    uint8  animType;
    uint32 frameSkipSpeed;
    bool   bothWay; // dovoluje animovat nejdriv k nejvyssimu a pak k nejnizsimu framu

    // Dynamicka data
    uint32 passedInterval;
    uint32 actualTextureId; // Kvuli rychlosti se bude uchovavat oboji
    uint32 actualFrame;
    bool   reversed;
    bool   disabled;
};

class Animator
{
    public:
        Animator();

        // System vyzvedavani, vyhledavani a znaceni ticketu
        uint32 GetTextureAnimTicket(uint32 textureId, uint32 animId, uint32 frameSkipSpeed = 1, bool bothWay = false);
        void DestroyAnimTicket(uint32 id);
        uint32 GetModelAnimTicket(uint32 modelId, uint32 animId, uint32 frameSkipSpeed = 1, bool bothWay = false);
        void ChangeModelAnim(uint32 ticketId, uint32 animId, uint32 startFrame = 0, uint32 frameSkipSpeed = 0, bool bothWay = false);
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
        void AnimateModelObjectByFrame(t3DObject *pObject, uint32 modelId, uint32 frame);
};

#define sCustomAnimator Singleton<CustomAnimator>::instance()

#endif
