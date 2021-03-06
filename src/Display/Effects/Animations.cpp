#include <Global.h>
#include <Effects/Animations.h>
#include <Storage.h>
#include <LoadingThread.h>
#include <SoundMgr.h>

/** \brief Konstruktor
 *
 * Pouze nulovani dat
 */
Animator::Animator()
{
    Anims.clear();
    m_lastUpdate = 0;
}

/** \brief Hlavni update funkce
 *
 * Zde se odehrava veskera magie animovani - projdou se vsechny tickety a pokud je nutne, posune se frame podle zadanych kriterii
 */
void Animator::Update()
{
    // Pokud je mapa prazdna, nema cenu pokracovat
    if (Anims.empty())
    {
        m_lastUpdate = 0;
        return;
    }

    clock_t tnow = clock();
    uint32 totalTicksCount = 0;

    // Ted projdeme vsechny zaznamy o animaci a pripadne posuneme frame

    AnimationRecord* temp = NULL;

    for (AnimMap::iterator itr = Anims.begin(); itr != Anims.end(); ++itr)
    {
        // Ulozit do temp promenne, abychom se z toho nezblaznili
        temp = &itr->second;

        if (temp->disabled)
            continue;

        if (temp->animType == ANIM_TYPE_TEXTURE)
        {
            if (temp->nextFrameTime <= tnow)
            {
                if (sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId][temp->actualFrame].interval > 0)
                    totalTicksCount = 1 + ((tnow - temp->nextFrameTime)/sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId][temp->actualFrame].interval);
                else
                    totalTicksCount = 1;

                // Nastavit casovac
                temp->nextFrameTime = tnow + sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId][temp->actualFrame].interval;

                if (temp->reversed)
                {
                    if (temp->actualFrame >= temp->frameSkipSpeed)
                    {
                        if (temp->actualFrame > (totalTicksCount * temp->frameSkipSpeed))
                            temp->actualFrame -= totalTicksCount * temp->frameSkipSpeed; // posunout frame zpet
                        else
                            temp->actualFrame = 0;
                    }
                    else
                        temp->actualFrame = 0;
                }
                else
                {
                    if ((temp->actualFrame + (totalTicksCount * temp->frameSkipSpeed)) < sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId].size())
                        temp->actualFrame += totalTicksCount * temp->frameSkipSpeed;     // posunout frame dopredu
                    else
                    {
                        if (!(temp->animFlags & ANIM_FLAG_NOT_REPEAT))
                            temp->actualFrame = sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId].size();
                        else
                            temp->actualFrame = sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId].size()-1;
                    }
                }

                // Pokud jsme se dostali na konec, opakovat animaci
                if (!temp->reversed && !(temp->animFlags & ANIM_FLAG_NOT_REPEAT) && sStorage->TextureAnimation[temp->sourceId].AnimFrameData[temp->animId].size() <= temp->actualFrame)
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
            if (temp->nextFrameTime <= tnow)
            {
                if (sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].interval > 0)
                    totalTicksCount = 1 + ((tnow - temp->nextFrameTime)/(sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].interval));
                else
                    totalTicksCount = 1;

                // Nastavit casovac
                temp->nextFrameTime = tnow + sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].interval;

                if (temp->reversed)
                {
                    if (temp->actualFrame >= temp->frameSkipSpeed)
                    {
                        if (int32(int32(temp->actualFrame) - int32((totalTicksCount * temp->frameSkipSpeed))) >= int32(sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst))
                            temp->actualFrame -= totalTicksCount * temp->frameSkipSpeed; // posunout frame zpet
                        else
                            temp->actualFrame = sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst;
                    }
                    else
                        temp->actualFrame = sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst;
                }
                else
                {
                    if (temp->actualFrame + (totalTicksCount * temp->frameSkipSpeed) < sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameLast)
                        temp->actualFrame += totalTicksCount * temp->frameSkipSpeed;     // posunout frame
                    else
                        temp->actualFrame = sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameLast;
                }

                // Pokud jsme na poslednim framu animace, opakujeme
                if (!temp->reversed && !(temp->animFlags & ANIM_FLAG_NOT_REPEAT) && sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameLast <= temp->actualFrame)
                {
                    if (temp->bothWay)
                    {
                        temp->actualFrame -= temp->frameSkipSpeed*2; // *2 pro netrhanou animaci
                        temp->reversed = true;
                    }
                    else
                        temp->actualFrame = sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst;
                }

                if (temp->reversed && temp->actualFrame == sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst)
                    temp->reversed = false;
            }
        }
    }
}

/** \brief Zjisteni ID animace podle dodaneho ID ticketu
 */
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

/** \brief Spusteni animace
 *
 * Zajisti spusteni (odpauzovani) animace podle ticketu
 */
void Animator::EnableAnimation(uint32 id)
{
    // Hleda ticket v mape
    AnimMap::iterator itr = Anims.find(id);
    // Pokud nenajde, vrati se
    if (itr == Anims.end())
        return;

    itr->second.disabled = false;
}

/** \brief Zastaveni animace
 *
 * Zastavi animovani daneho ticketu
 */
void Animator::DisableAnimation(uint32 id)
{
    // Hleda ticket v mape
    AnimMap::iterator itr = Anims.find(id);
    // Pokud nenajde, vrati se
    if (itr == Anims.end())
        return;

    itr->second.disabled = true;
}

/** \brief Zjisteni aktualni textury
 *
 * Zjisti aktualni texturu podle dodaneho ID ticketu
 *
 * Tahle metoda je bezpecnejsi nez vraceni celeho zaznamu, protoze potrebujeme, aby
 * se o zaznam v mape staral jen a pouze nas Animator - zamezi jakemukoliv leaku,
 * nebo vymazani zaznamu pred ziskavanim dat v jinem vlakne
 */
uint32 Animator::GetActualTexture(uint32 id)
{
    // Hleda ticket v mape
    AnimMap::const_iterator itr = Anims.find(id);
    // Pokud nenajde, vrati nulu
    if (itr == Anims.end())
        return 0;

    // Jinak vrati platnou hodnotu
    return itr->second.actualTextureId;
}

/** \brief Zjisteni aktualniho snimku
 *
 * Vraci frame animace podle daneho ticketu
 */
uint32 Animator::GetActualFrame(uint32 id)
{
    AnimMap::const_iterator itr = Anims.find(id);
    if (itr == Anims.end())
        return 0;

    return itr->second.actualFrame;
}

/** \brief Zniceni zaznamu animace
 *
 * Bezpecne zahodi veskera animacni data spojena s danym ticketem
 */
void Animator::DestroyAnimTicket(uint32 id)
{
    AnimMap::const_iterator itr = Anims.find(id);
    if (itr == Anims.end())
        return;

    Anims.erase(itr);
}

/** \brief Zjisteni nejnizsiho pouzitelneho (volneho) ID ticketu
 *
 * Projde vsechny animace a zjisti "diry" v jejich cislovani
 */
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

/** \brief Vyzadani ticketu pro animaci textury
 *
 * Postara se o prirazeni ID ticketu, pote vytvori animacni zaznam, priradi tento ticket k nemu a zpatky vrati
 * jen ID ticketu pro pripadne dalsi akce
 */
uint32 Animator::GetTextureAnimTicket(uint32 textureId, uint32 animId, uint32 frameSkipSpeed, uint8 flags)
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
    temp->nextFrameTime   = clock();
    temp->frameSkipSpeed  = (frameSkipSpeed > 0) ? frameSkipSpeed : 1;
    temp->bothWay         = (flags & ANIM_FLAG_BOTHWAY);
    temp->animFlags       = flags;

    if (flags & ANIM_FLAG_FORCE_LOADING)
    {
        for (std::vector<std::vector<TextureAnim::FrameData>>::const_iterator itr = sStorage->TextureAnimation[textureId].AnimFrameData.begin()
             ;itr != sStorage->TextureAnimation[textureId].AnimFrameData.end(); ++itr)
        {
            for (std::vector<TextureAnim::FrameData>::const_iterator ittr = (*itr).begin(); ittr != (*itr).end(); ++ittr)
            {
                sLoader->RequestLoadBlocking(LOAD_TEXTURE, (*ittr).textureId);
            }
        }
    }

    return pos;
}

/** \brief Vyzadani ticketu pro animaci modelu
 *
 * Postara se o prirazeni ID ticketu, pote vytvori animacni zaznam, priradi tento ticket k nemu a zpatky vrati
 * jen ID ticketu pro pripadne dalsi akce
 */
uint32 Animator::GetModelAnimTicket(uint32 modelId, uint32 animId, uint32 frameSkipSpeed, uint8 flags)
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
    temp->nextFrameTime   = clock();
    temp->frameSkipSpeed  = (frameSkipSpeed > 0) ? frameSkipSpeed : 1;
    temp->bothWay         = (flags & ANIM_FLAG_BOTHWAY);
    temp->animFlags       = flags;

    return pos;
}

/** \brief Zmena ID animace danemu ticketu
 *
 * Postara se o zmenu animace, framu, rychlosti a priznaku podle vstupnich argumentu
 */
void Animator::ChangeModelAnim(uint32 ticketId, uint32 animId, uint32 startFrame, uint32 frameSkipSpeed, uint8 flags)
{
    if (animId > MAX_ANIM || ticketId == 0)
        return;

    AnimMap::iterator itr = Anims.find(ticketId);
    if (itr == Anims.end() || itr->second.animType != ANIM_TYPE_MODEL)
        return;

    itr->second.animId         = animId;
    itr->second.actualFrame    = sStorage->ModelAnimation[itr->second.sourceId].Anim[animId].frameFirst + startFrame;
    itr->second.nextFrameTime  = clock();
    itr->second.bothWay        = (flags & ANIM_FLAG_BOTHWAY);
    itr->second.animFlags      = flags;
    if (frameSkipSpeed > 0)
        itr->second.frameSkipSpeed = frameSkipSpeed;
}

/** \brief Zmena rychlosti animace podle tiketu
 *
 * Stara se o zmenu rychlost animace (frame skip)
 */
void Animator::ChangeModelAnimFrameSpeed(uint32 ticketId, uint32 frameSkipSpeed)
{
    if (ticketId == 0)
        return;

    AnimMap::iterator itr = Anims.find(ticketId);
    if (itr == Anims.end() || itr->second.animType != ANIM_TYPE_MODEL)
        return;

    itr->second.frameSkipSpeed = frameSkipSpeed;
}

/** \brief Konstruktor
 *
 * Prazdny
 */
CustomAnimator::CustomAnimator()
{
}

/** \brief Zjisteni, zdali ma model custom animaci
 *
 * Neciste reseni, ale v takto malem meritku se ztrati. Jde o "hardcoded" hodnoty pro par modelu, ktere potrebujeme animovat variabilne k situaci,
 * nebo chceme obejit nejake zavedene zakonitosti.
 */
bool CustomAnimator::HaveModelCustomAnim(uint32 id)
{
    // TODO: odebrat staticke zaznamy, nahradit necim dynamickym !!!

    // Model bomby
    if (id == 8)
        return true;
    // Model hlavni postavy + headless postavy (meme mod)
    if (id == 9 || id == 11)
        return true;
    // enemy-1.3DS, enemy-2.3DS
    if (id == 10 || id == 12)
        return true;

    return false;
}

/** \brief Vlastni animace podle zadaneho framu
 *
 * Magicka funkce, majici za ukol zastirat programatorovu neschopnost a obchazet absenci kompletni dokumentace formatu 3DS
 *
 * Trocha vysvetleni
 *
 * Ve skutecnosti jde o neco, co dokaze animovat objekty nebo skupinu objektu (viz. vicecetne strcmp v podminkach a animace
 * podle pozice jineho objektu) podle relativniho stredu nebo stredu jineho objektu. To nam dovoluje obejit veskery pivot
 * mechanizmus obsazeny primo ve formatu 3DS. Tady se tedy deje veskera magie, bohuzel se mi nechtelo patlat s kombinaci
 * s puvodnim animacnim systemem, proto to vypada tak jak to vypada, tzn. bud built-in animace nebo tahle staticka hnusna
 *
 * Budoucimu ja preji pevne nervy pri cteni a lusteni tohoto kodu a take prosim o trochu pochopeni.
 *
 *
 * Popis magie:
 *
 * Zpravidla se nejdrive najde objekt, jehoz stred se bude povazovat za vychozi (FindModelObject, ..)
 * Bod rotPos (deklarovany jako vektor, ale ne vsechno co ma tri souradnice je vektor, ze..) dokaze oznacit stred otaceni,
 * ktery je polozen relativne vuci pocatku celeho modelu.
 * Pote se matrix posune do danych souradnic a objekt se zvetsi/zmensi/zrotuje jak je treba.
 * Nakonec se souradnice prelozi zpet na puvodni pozici a nasledne hned na pozici puvodniho objektu, aby se nevykreslovalo
 * relativne k objektu, jehoz pozice vyuzivame jen k pootoceni.
 *
 * Co se tyce "vzorecku" pro rotaci napr. rukou postavy atp.:
 * Cely vzorec sestava z nekolika zakladnich prvku, ktere se daji defakto identifikovat podle standardniho vzorce linearni
 * funkce s absolutni hodnotou:
 *
 * y = -| x/{nejvyssi_bod} - 1 | + {bod_zlomu}
 *
 * nejvyssi bod je takovy, ve kterem ma jednak dana funkce maximum, a jednak je to frame animace s nejvyssi hodnotou rotace.
 * Pri posunu animace se samozrejme musi posunout i "x", aby "zacinalo od nuly" a ne od prvniho framu animace.
 * bod zlomu je takovy bod, ve kterem se meni znamenko, tedy pokud z absolutni hodnoty "padaji" cisla v intervalu <0;1>,
 * tento bod zaruci jakysi posun. Viz napr. pohyb ruky, kdy je treba nejdrive rotovat do plusu a pak do minusu.
 */
void CustomAnimator::AnimateModelObjectByFrame(t3DObject *object, uint32 modelId, uint32 frame)
{
    AnimateModelObjectByFrame(object, sStorage->Models[modelId], modelId, frame);
}

void CustomAnimator::AnimateModelObjectByFrame(t3DObject* object, t3DModel* model, uint32 modelId, uint32 frame)
{
    AnimateModelObjectByFrame(object, model, modelId, frame, sStorage->GetAnimTypeForFrame(modelId, frame));
}

void CustomAnimator::AnimateModelObjectByFrame(t3DObject *object, t3DModel* model, uint32 modelId, uint32 frame, ModelAnimType anim)
{
    bool processed = false;

    if (!model || !object)
        return;

    uint32 realFrame = frame;
    if (object->vPosition.size() <= frame)
        realFrame = 1;

    // Model hrace - chuze
    if ((modelId == 9 || modelId == 11) && anim == ANIM_WALK)
    {
        float wholePosMod = 1.0f-fabs((float(frame%50)/25.0f)-1.0f);
        wholePosMod = cos(wholePosMod*PI/2);
        glTranslatef(0, wholePosMod*0.2f, 0);

        if (strcmp(object->strName, "Hlava") == 0 || strcmp(object->strName, "Oci") == 0 || strcmp(object->strName, "Antena") == 0
             || strcmp(object->strName, "AntenaS") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "Hlava");
            if (!pObj)
                return;

            // Rotaci budeme resit podle sdileneho stredu otaceni hlavy
            CVector3 vPosition = pObj->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.5f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*35.0f, 0, 1.0f, 0);

            // Po otoceni prelozime zpaky na puvodni pozice a prelozime na pozice objektu pro korektni zobrazeni
            glTranslatef(-vPosition.x, -vPosition.y, -vPosition.z);
            vPosition = object->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        if (strcmp(object->strName, "Telo") == 0 || strcmp(object->strName, "Pasek") == 0 || strcmp(object->strName, "Preska") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "Telo");
            if (!pObj)
                return;

            // Rotaci budeme resit podle sdileneho stredu otaceni hlavy
            CVector3 vPosition = pObj->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.5f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*15.0f, 0, 1.0f, 0);

            // Po otoceni prelozime zpaky na puvodni pozice a prelozime na pozice objektu pro korektni zobrazeni
            glTranslatef(-vPosition.x, -vPosition.y, -vPosition.z);
            vPosition = object->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(object->strName, "LevaRuka") == 0 || strcmp(object->strName, "LevaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "LevaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(-0.173f, 1.353f, 0.08f);

            CVector3 vPosition = object->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.6f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(50.0f, 0, 0, 1.0f);
            glRotatef(anglemod*80.0f, 0, 1.0f, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(object->strName, "PravaRuka") == 0 || strcmp(object->strName, "PravaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "PravaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(0.173f, 1.353f, 0.08f);

            CVector3 vPosition = object->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.6f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(50.0f, 0, 0, -1.0f);
            glRotatef(anglemod*80.0f, 0, 1.0f, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(object->strName, "LevaNoha") == 0 || strcmp(object->strName, "LeveCh") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "LevaNoha");
            if (!pObj)
                return;

            CVector3 rotPos(-0.118f, 0.761f, 0.083f);

            CVector3 vPosition = object->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.45f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*75.0f, 1.0f, 0, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(object->strName, "PravaNoha") == 0 || strcmp(object->strName, "PraveCh") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "PravaNoha");
            if (!pObj)
                return;

            CVector3 rotPos(0.118f, 0.761f, 0.083f);

            CVector3 vPosition = object->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.45f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*75.0f, -1.0f, 0, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
    }
    // Model hrace - zevleni
    else if ((modelId == 9 || modelId == 11) && anim == ANIM_IDLE)
    {
        if (strcmp(object->strName, "LevaRuka") == 0 || strcmp(object->strName, "LevaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "LevaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(-0.173f, 1.353f, 0.08f);

            CVector3 vPosition = object->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            glRotatef(50.0f, 0, 0, 1.0f);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(object->strName, "PravaRuka") == 0 || strcmp(object->strName, "PravaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "PravaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(0.173f, 1.353f, 0.08f);

            CVector3 vPosition = object->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = object->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            glRotatef(50.0f, 0, 0, -1.0f);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
    }
    // Model hrace - smrt
    else if ((modelId == 9 || modelId == 11) && anim == ANIM_DYING)
    {
        if (strcmp(object->strName, "Hlava") == 0 || strcmp(object->strName, "Oci") == 0 || strcmp(object->strName, "Antena") == 0
             || strcmp(object->strName, "AntenaS") == 0 || strcmp(object->strName, "PravaRuka") == 0 || strcmp(object->strName, "PravaDlan") == 0
             || strcmp(object->strName, "LevaRuka") == 0 || strcmp(object->strName, "LevaDlan") == 0 || strcmp(object->strName, "PravaNoha") == 0
             || strcmp(object->strName, "PraveCh") == 0 || strcmp(object->strName, "LevaNoha") == 0 || strcmp(object->strName, "LeveCh") == 0
             || strcmp(object->strName, "Telo") == 0 || strcmp(object->strName, "Pasek") == 0 || strcmp(object->strName, "Preska") == 0)
        {
            CVector3 vPosition = object->vPosition[realFrame];
            glTranslatef(vPosition.x*(1.0f+(float(frame-102)/100)), vPosition.y*cos(((2.0f*float(frame-102)/100.0f)-1.0f)*(PI/2)), vPosition.z*(1.0f+(float(frame-102)/100)));
            CVector3 vScale = object->vScale[realFrame];
            glScalef(vScale.x*(1.0f-(float(frame-102)/150.0f)), vScale.y*(1.0f-(float(frame-102)/150.0f)), vScale.z*(1.0f-(float(frame-102)/150.0f)));

            processed = true;
        }
    }
    // Model bomby - tikani
    else if (modelId == 8 && anim == ANIM_IDLE)
    {
        // Bomba se bude zmensovat a zvetsovat, jako v Dyna Blasterovi

        t3DObject* pObj = sStorage->FindModelObjectInNonStored(model, "Sphere01");
        if (!pObj)
            return;

        CVector3 vPosition = object->vPosition[realFrame];
        glTranslatef(pObj->vPosition[realFrame].x, pObj->vPosition[realFrame].y, pObj->vPosition[realFrame].z);
        CVector3 vScale = object->vScale[realFrame];
        glScalef(vScale.x, vScale.y, vScale.z);

        float scalingFactor = (1.0f + (frame/100.0f)*0.3f);
        glScalef(scalingFactor, scalingFactor, scalingFactor);
        model->customScale[realFrame] = scalingFactor;

        glTranslatef(-pObj->vPosition[realFrame].x, -pObj->vPosition[realFrame].y, -pObj->vPosition[realFrame].z);
        glTranslatef(vPosition.x, vPosition.y, vPosition.z);

        processed = true;
    }
    // Sdilena DYING (umiraci) animace pro enemy modely, nejspis bude u hodne modelu stejna
    else if (anim == ANIM_DYING)
    {
        // Nalezeni objektu slouziciho jako ridici (podle jeho stredu se budou otacet vsechny objekty)
        t3DObject* pObj = NULL;
        if (modelId == 10)
            pObj = sStorage->FindModelObjectInNonStored(model, "Loft01");
        else if (modelId == 12)
            pObj = sStorage->FindModelObjectInNonStored(model, "Voko");

        // Pokud byl nalezen, pootocit a sundat na zem
        if (pObj)
        {
            CVector3 vPosition = object->vPosition[realFrame];
            glTranslatef(pObj->vPosition[realFrame].x, pObj->vPosition[realFrame].y-(8.0f*float(frame)/100.0f), pObj->vPosition[realFrame].z);

            glRotatef((float(frame)/100.0f)*90.0f, -1.0f, 0.0f, 0.0f);
            float factor = (1.0f-(float(frame)/100.0f))*0.5f+0.5f;
            glScalef(factor, factor, factor);

            glTranslatef(-pObj->vPosition[realFrame].x, -pObj->vPosition[realFrame].y, -pObj->vPosition[realFrame].z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
    }

    // Pokud se "nechytnul" ani jeden z nami deklarovanych objektu na danou animaci, vyuzijeme standardni postupy
    if (!processed)
    {
        CVector3 vPosition = object->vPosition[realFrame];
        glTranslatef(vPosition.x, vPosition.y, vPosition.z);
        CVector3 vScale = object->vScale[realFrame];
        glScalef(vScale.x, vScale.y, vScale.z);

        for (uint32 i = 1; i <= realFrame; i++)
        {
            CVector3 vRotation = object->vRotation[i];
            float rotDegree = object->vRotDegree[i];

            if(rotDegree)
                glRotatef(rotDegree, vRotation.x, vRotation.y, vRotation.z);
        }
    }
}

/** \overload void CustomAnimator::AnimateModelObjectByFrame(t3DObject* object, t3DModel* model, uint32 modelId, uint32 frame)
 *
 */

/** \overload void CustomAnimator::AnimateModelObjectByFrame(t3DObject *object, t3DModel* model, uint32 modelId, uint32 frame, ModelAnimType anim)
 *
 */

/** \brief Vlastni animace modelovych featur
 *
 * Jde jen o jejich presun k displaylist zaznamu pro model a nekde i o vlastni animaci (posun, rotaci, ..)
 */
void CustomAnimator::AnimateModelFeatures(ModelDisplayListRecord *record)
{
    if (!record || record->features.empty())
        return;

    t3DModel* pModel = sStorage->Models[record->modelId];

    if (!pModel)
        return;

    uint32 frame = sAnimator->GetActualFrame(record->AnimTicket);
    uint32 realFrame = frame;
    if (realFrame >= pModel->customScale.size())
        realFrame = 1;

    // Tikajici bomba - posunovani emitteru
    if (record->modelId == 8 && sAnimator->GetAnimId(record->AnimTicket) == ANIM_IDLE)
    {
        int32 delta = float(clock() - record->insertionTime)*1.3f;
        record->CustomFrame = (delta);

        for (FeatureList::iterator itr = record->features.begin(); itr != record->features.end(); ++itr)
        {
            if ((*itr)->type == MF_TYPE_EMITTER)
            {
                (*itr)->offset_x = 17.4f-0.0037f*delta;
                (*itr)->offset_y = 18.539f+0.0015f*delta;
                (*itr)->offset_z = 21.02f-0.0042f*delta;
            }
        }
    }

    // Posunuti vsech model featur podle nadrazeneho modelu
    for (FeatureList::iterator itr = record->features.begin(); itr != record->features.end(); ++itr)
    {
        switch ((*itr)->type)
        {
            case MF_TYPE_MODEL:
                (*itr)->ToModel()->x = record->x + ((*itr)->offset_x * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                (*itr)->ToModel()->y = record->y + ((*itr)->offset_y * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                (*itr)->ToModel()->z = record->z + ((*itr)->offset_z * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                break;
            case MF_TYPE_BILLBOARD:
                (*itr)->ToBillboard()->x = record->x + ((*itr)->offset_x * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                (*itr)->ToBillboard()->y = record->y + ((*itr)->offset_y * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                (*itr)->ToBillboard()->z = record->z + ((*itr)->offset_z * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);

                // Headless bomberman
                if (record->modelId == 11)
                {
                    if (sAnimator->GetAnimId(record->AnimTicket) == ANIM_WALK)
                        (*itr)->ToBillboard()->y += (cos((1.0f-fabs((float(frame%50)/25.0f)-1.0f))*PI/2))*0.1f;
                    else if (sAnimator->GetAnimId(record->AnimTicket) == ANIM_DYING)
                    {
                        (*itr)->ToBillboard()->y -= float(frame-102)/130.0f;
                        (*itr)->ToBillboard()->billboard_x = false;
                        (*itr)->ToBillboard()->billboard_y = false;
                        (*itr)->ToBillboard()->rotate_x = (90.0f*float(frame-102)/100.0f);
                        (*itr)->ToBillboard()->rotate_z = (90.0f*float(frame-102)/100.0f);
                    }
                }
                break;
            case MF_TYPE_EMITTER:
                (*itr)->ToEmitter()->m_centerX = record->x + ((*itr)->offset_x * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                (*itr)->ToEmitter()->m_centerY = record->y + ((*itr)->offset_y * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                (*itr)->ToEmitter()->m_centerZ = record->z + ((*itr)->offset_z * MODEL_SCALE * record->scale * pModel->customScale[realFrame]);
                break;
            default:
                break;
        }
    }
}
