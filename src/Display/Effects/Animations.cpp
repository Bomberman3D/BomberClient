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

        if (temp->disabled)
            continue;

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
                        temp->actualFrame = sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst;
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
                        temp->actualFrame = sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst;
                }

                if (temp->reversed && temp->actualFrame == sStorage->ModelAnimation[temp->sourceId].Anim[temp->animId].frameFirst)
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

void Animator::EnableAnimation(uint32 id)
{
    // Hleda ticket v mape
    AnimMap::iterator itr = Anims.find(id);
    // Pokud nenajde, vrati se
    if (itr == Anims.end())
        return;

    itr->second.disabled = false;
}

void Animator::DisableAnimation(uint32 id)
{
    // Hleda ticket v mape
    AnimMap::iterator itr = Anims.find(id);
    // Pokud nenajde, vrati se
    if (itr == Anims.end())
        return;

    itr->second.disabled = true;
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

CustomAnimator::CustomAnimator()
{
}

bool CustomAnimator::HaveModelCustomAnim(uint32 id)
{
    // TODO: odebrat staticke zaznamy, nahradit necim dynamickym !!!

    // Model hlavni postavy
    if (id == 9)
        return true;

    return false;
}

void CustomAnimator::AnimateModelObjectByFrame(t3DObject *pObject, ModelDisplayListRecord *model, uint32 frame)
{
    /*  Magicka funkce, majici za ukol zastirat programatorovu neschopnost a obchazet absenci kompletni dokumentace formatu 3DS
     *  Trocha vysvetleni
     *
     *  Ve skutecnosti jde o neco, co dokaze animovat objekty nebo skupinu objektu (viz. vicecetne strcmp v podminkach a animace
     *  podle pozice jineho objektu) podle relativniho stredu nebo stredu jineho objektu. To nam dovoluje obejit veskery pivot
     *  mechanizmus obsazeny primo ve formatu 3DS. Tady se tedy deje veskera magie, bohuzel se mi nechtelo patlat s kombinaci
     *  s puvodnim animacnim systemem, proto to vypada tak jak to vypada, tzn. bud built-in animace nebo tahle staticka hnusna
     *
     *  Budoucimu ja preji pevne nervy pri cteni a lusteni tohoto kodu a take prosim o trochu pochopeni.
     */

    /*
     * Trochu popis:
     *
     *  Zpravidla se nejdrive najde objekt, jehoz stred se bude povazovat za vychozi (FindModelObject, ..)
     *  Bod rotPos (deklarovany jako vektor, ale ne vsechno co ma tri souradnice je vektor, ze..) dokaze oznacit stred otaceni,
     *  ktery je polozen relativne vuci pocatku celeho modelu.
     *  Pote se matrix posune do danych souradnic a objekt se zvetsi/zmensi/zrotuje jak je treba.
     *  Nakonec se souradnice prelozi zpet na puvodni pozici a nasledne hned na pozici puvodniho objektu, aby se nevykreslovalo
     *  relativne k objektu, jehoz pozice vyuzivame jen k pootoceni.
     *
     *  Co se tyce "vzorecku" pro rotaci napr. rukou postavy atp.:
     *  Cely vzorec sestava z nekolika zakladnich prvku, ktere se daji defakto identifikovat podle standardniho vzorce linearni
     *  funkce s absolutni hodnotou:
     *
     *  y = -| x/{nejvyssi_bod} - 1 | + {bod_zlomu}
     *
     *  nejvyssi bod je takovy, ve kterem ma jednak dana funkce maximum, a jednak je to frame animace s nejvyssi hodnotou rotace.
     *  Pri posunu animace se samozrejme musi posunout i "x", aby "zacinalo od nuly" a ne od prvniho framu animace.
     *  bod zlomu je takovy bod, ve kterem se meni znamenko, tedy pokud z absolutni hodnoty "padaji" cisla v intervalu <0;1>,
     *  tento bod zaruci jakysi posun. Viz napr. pohyb ruky, kdy je treba nejdrive rotovat do plusu a pak do minusu.
     */
    bool processed = false;

    if (model->modelId == 9 && sStorage->GetAnimTypeForFrame(model->modelId, frame) == ANIM_WALK)
    {
        float wholePosMod = 1.0f-fabs((float(frame%50)/25.0f)-1.0f);
        wholePosMod = cos(wholePosMod*PI/2);
        glTranslatef(0, wholePosMod*0.2f, 0);

        if (strcmp(pObject->strName, "Hlava") == 0 || strcmp(pObject->strName, "Oci") == 0 || strcmp(pObject->strName, "Antena") == 0
             || strcmp(pObject->strName, "AntenaS") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "Hlava");
            if (!pObj)
                return;

            // Rotaci budeme resit podle sdileneho stredu otaceni hlavy
            CVector3 vPosition = pObj->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.5f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*35.0f, 0, 1.0f, 0);

            // Po otoceni prelozime zpaky na puvodni pozice a prelozime na pozice objektu pro korektni zobrazeni
            glTranslatef(-vPosition.x, -vPosition.y, -vPosition.z);
            vPosition = pObject->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        if (strcmp(pObject->strName, "Telo") == 0 || strcmp(pObject->strName, "Pasek") == 0 || strcmp(pObject->strName, "Preska") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "Telo");
            if (!pObj)
                return;

            // Rotaci budeme resit podle sdileneho stredu otaceni hlavy
            CVector3 vPosition = pObj->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.5f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*15.0f, 0, 1.0f, 0);

            // Po otoceni prelozime zpaky na puvodni pozice a prelozime na pozice objektu pro korektni zobrazeni
            glTranslatef(-vPosition.x, -vPosition.y, -vPosition.z);
            vPosition = pObject->vPosition[frame];
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(pObject->strName, "LevaRuka") == 0 || strcmp(pObject->strName, "LevaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "LevaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(-0.173f, 1.353f, 0.08f);

            CVector3 vPosition = pObject->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.6f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(50.0f, 0, 0, 1.0f);
            glRotatef(anglemod*80.0f, 0, 1.0f, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(pObject->strName, "PravaRuka") == 0 || strcmp(pObject->strName, "PravaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "PravaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(0.173f, 1.353f, 0.08f);

            CVector3 vPosition = pObject->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.6f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(50.0f, 0, 0, -1.0f);
            glRotatef(anglemod*80.0f, 0, 1.0f, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(pObject->strName, "LevaNoha") == 0 || strcmp(pObject->strName, "LeveCh") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "LevaNoha");
            if (!pObj)
                return;

            CVector3 rotPos(-0.118f, 0.761f, 0.083f);

            CVector3 vPosition = pObject->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.45f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*75.0f, 1.0f, 0, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(pObject->strName, "PravaNoha") == 0 || strcmp(pObject->strName, "PraveCh") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "PravaNoha");
            if (!pObj)
                return;

            CVector3 rotPos(0.118f, 0.761f, 0.083f);

            CVector3 vPosition = pObject->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            float anglemod = 0.45f-fabs(((float(frame))/50.0f)-1.0f);

            glRotatef(anglemod*75.0f, -1.0f, 0, 0);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
    }
    else if (model->modelId == 9 && sStorage->GetAnimTypeForFrame(model->modelId, frame) == ANIM_IDLE)
    {
        if (strcmp(pObject->strName, "LevaRuka") == 0 || strcmp(pObject->strName, "LevaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "LevaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(-0.173f, 1.353f, 0.08f);

            CVector3 vPosition = pObject->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            glRotatef(50.0f, 0, 0, 1.0f);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
        else if (strcmp(pObject->strName, "PravaRuka") == 0 || strcmp(pObject->strName, "PravaDlan") == 0)
        {
            t3DObject* pObj = sStorage->FindModelObject(model->modelId, "PravaRuka");
            if (!pObj)
                return;

            CVector3 rotPos(0.173f, 1.353f, 0.08f);

            CVector3 vPosition = pObject->vPosition[frame];
            glTranslatef(rotPos.x, rotPos.y, rotPos.z);
            CVector3 vScale = pObject->vScale[frame];
            glScalef(vScale.x, vScale.y, vScale.z);

            glRotatef(50.0f, 0, 0, -1.0f);

            glTranslatef(-rotPos.x, -rotPos.y, -rotPos.z);
            glTranslatef(vPosition.x, vPosition.y, vPosition.z);

            processed = true;
        }
    }

    // Pokud se "nechytnul" ani jeden z nami deklarovanych objektu na danou animaci, vyuzijeme standardni postupy
    if (!processed)
    {
        CVector3 vPosition = pObject->vPosition[frame];
        glTranslatef(vPosition.x, vPosition.y, vPosition.z);
        CVector3 vScale = pObject->vScale[frame];
        glScalef(vScale.x, vScale.y, vScale.z);

        for (uint32 i = 1; i <= frame; i++)
        {
            CVector3 vRotation = pObject->vRotation[i];
            float rotDegree = pObject->vRotDegree[i];

            if(rotDegree)
                glRotatef(rotDegree, vRotation.x, vRotation.y, vRotation.z);
        }
    }
}
