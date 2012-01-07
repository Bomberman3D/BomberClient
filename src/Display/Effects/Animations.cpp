#include <Global.h>
#include <Effects/Animations.h>
#include <Storage.h>

Animator::Animator()
{
    Anims.clear();
    m_lastUpdate = 0;
}

void Animator::Update()
{
    // Pokud je mapa prazdna, nema cenu pokracovat
    if (Anims.empty())
    {
        m_lastUpdate = 0;
        return;
    }

    // Nejdrive si musime vyridit diff pro update
    if (m_lastUpdate == 0)
        m_diff = 0;
    else
        m_diff = clock() - m_lastUpdate;

    m_lastUpdate = clock();

    // Ted projdeme vsechny zaznamy o animaci a pripadne posuneme frame

    AnimationRecord* temp = NULL;

    for (AnimMap::iterator itr = Anims.begin(); itr != Anims.end(); ++itr)
    {
        // Ulozit do temp promenne, abychom se z toho nezblaznili
        temp = &itr->second;

        temp->passedInterval += m_diff;

        if (temp->animType == ANIM_TYPE_TEXTURE)
        {
            if (temp->passedInterval >= sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId][temp->actualFrame].interval)
            {
                temp->passedInterval = 0;                    // Resetovat casovac

                if (temp->reversed)
                {
                    if (temp->actualFrame >= temp->frameSkipSpeed)
                        temp->actualFrame -= temp->frameSkipSpeed; // posunout frame zpet
                    else
                        temp->actualFrame = 0;
                }
                else
                    temp->actualFrame += temp->frameSkipSpeed; // posunout frame dopredu

                // Pokud jsme se dostali na konec, opakovat animaci
                if (!temp->reversed && sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId].size() <= temp->actualFrame)
                {
                    if (temp->bothWay)
                    {
                        temp->actualFrame -= temp->frameSkipSpeed*2; // *2 pro netrhanou animaci
                        temp->reversed = true;
                    }
                    else
                        temp->actualFrame = 0;
                }

                if (temp->reversed && temp->actualFrame == 0)
                    temp->reversed = false;

                // A nakonec ulozit texturu
                temp->actualTextureId = sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId][temp->actualFrame].textureId;
            }
        }
        // prakticky by nemel nastat pripad, kdy se animType rovna necemu jinemu, ale ja se radsi pojistim
        else if (temp->animType == ANIM_TYPE_MODEL)
        {
            if (temp->passedInterval >= sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].interval)
            {
                temp->passedInterval = 0;                    // Resetovat casovac

                if (temp->reversed)
                {
                    if (temp->actualFrame >= temp->frameSkipSpeed)
                        temp->actualFrame -= temp->frameSkipSpeed; // posunout frame zpet
                    else
                        temp->actualFrame = 0;
                }
                else
                    temp->actualFrame += temp->frameSkipSpeed; // posunout frame

                // Pokud jsme na poslednim framu animace, opakujeme
                if (!temp->reversed && sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameLast <= temp->actualFrame)
                {
                    if (temp->bothWay)
                    {
                        temp->actualFrame -= temp->frameSkipSpeed*2; // *2 pro netrhanou animaci
                        temp->reversed = true;
                    }
                    else
                        temp->actualFrame = 0;
                }

                if (temp->reversed && temp->actualFrame == 0)
                    temp->reversed = false;
            }
        }
    }
}

uint32 Animator::GetAnimId(uint32 id)
{
    // Hleda ticket v mape
    AnimMap::const_iterator itr = Anims.find(id);
    // Pokud nenajde, vrati nulu
    if (itr == Anims.end())
        return 0;

    // Jinak vrati platnou hodnotu
    return itr->second.animId;
}

uint32 Animator::GetActualTexture(uint32 id)
{
    /* Tahle metoda je bezpecnejsi nez vraceni celeho zaznamu, protoze potrebujeme, aby
     * se o zaznam v mape staral jen a pouze nas Animator - zamezi jakemukoliv leaku,
     * nebo vymazani zaznamu pred ziskavanim dat v jinem vlakne
     */

    // Hleda ticket v mape
    AnimMap::const_iterator itr = Anims.find(id);
    // Pokud nenajde, vrati nulu
    if (itr == Anims.end())
        return 0;

    // Jinak vrati platnou hodnotu
    return itr->second.actualTextureId;
}

uint32 Animator::GetActualFrame(uint32 id)
{
    AnimMap::const_iterator itr = Anims.find(id);
    if (itr == Anims.end())
        return 0;

    return itr->second.actualFrame;
}

void Animator::DestroyAnimTicket(uint32 id)
{
    AnimMap::const_iterator itr = Anims.find(id);
    if (itr == Anims.end())
        return;

    Anims.erase(itr);
}

uint32 Animator::GetLowestAnimTicketId()
{
    // Projde vsechny zaznamy od 0 do nejakeho silene vysokeho cisla jako je INT_MAX ( = 2^31-1 )
    for (uint32 i = 1; i < INT_MAX; i++)
    {
        // Pokud zaznam s timto ticket id nenajde, vrati tuhle hodnotu jako pouzitelnou
        if (Anims.find(i) == Anims.end())
            return i;
    }

    return (uint32)-1;
}

uint32 Animator::GetTextureAnimTicket(uint32 textureId, uint32 animId, uint32 frameSkipSpeed, bool bothWay)
{
    // Ziska nejnizsi mozne ID ticketu
    uint32 pos = GetLowestAnimTicketId();
    // Pokud neni volno, vrati chybu
    if (pos == (uint32)-1)
        return pos;

    AnimationRecord* temp = &Anims[pos];

    // Naplneni daty
    temp->animType        = ANIM_TYPE_TEXTURE;
    temp->sourceId        = textureId;
    temp->animId          = animId;
    temp->actualTextureId = textureId;
    temp->actualFrame     = 0;
    temp->passedInterval  = 0;
    temp->frameSkipSpeed  = (frameSkipSpeed > 0) ? frameSkipSpeed : 1;
    temp->bothWay         = bothWay;

    return pos;
}

uint32 Animator::GetModelAnimTicket(uint32 modelId, uint32 animId, uint32 frameSkipSpeed, bool bothWay)
{
    // Pokud vubec muzeme animovat
    if (animId > MAX_ANIM)
        return ((uint32)-1);

    // Ziska nejnizsi mozne ID ticketu
    uint32 pos = GetLowestAnimTicketId();
    // Pokud neni volno, vrati chybu
    if (pos == (uint32)-1)
        return pos;

    AnimationRecord* temp = &Anims[pos];

    // Naplneni daty
    temp->animType        = ANIM_TYPE_MODEL;
    temp->sourceId        = modelId;
    temp->animId          = animId;
    temp->actualTextureId = 0;
    temp->actualFrame     = sStorage->ModelAnimation[modelId].Anim[animId].frameFirst;
    temp->passedInterval  = 0;
    temp->frameSkipSpeed  = (frameSkipSpeed > 0) ? frameSkipSpeed : 1;
    temp->bothWay         = bothWay;

    return pos;
}

void Animator::ChangeModelAnim(uint32 ticketId, uint32 animId, uint32 startFrame, uint32 frameSkipSpeed, bool bothWay)
{
    if (animId > MAX_ANIM || ticketId == 0)
        return;

    AnimMap::iterator itr = Anims.find(ticketId);
    if (itr == Anims.end() || itr->second.animType != ANIM_TYPE_MODEL)
        return;

    itr->second.animId         = animId;
    itr->second.actualFrame    = sStorage->ModelAnimation[itr->second.sourceId].Anim[animId].frameFirst + startFrame;
    itr->second.passedInterval = 0;
    itr->second.bothWay        = bothWay;
    if (frameSkipSpeed > 0)
        itr->second.frameSkipSpeed = frameSkipSpeed;
}
