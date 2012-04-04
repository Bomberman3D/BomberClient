#include <Global.h>
#include <Helpers.h>
#include <Display.h>
#include <Effects/Animations.h>
#include <Effects/ParticleEmitter.h>
#include <Map.h>
#include <LoadingThread.h>

Display::Display()
{
    ModelDisplayList.clear();
    BillboardDisplayList.clear();

    m_boundTexture = (uint32)-1;

    for (uint8 i = 0; i < MAX_FONTS; i++)
        m_fontLoaded[i] = false;

    m_ignoreTargetCollision = 0;
}

Display::~Display()
{
    for (uint8 i = 0; i < MAX_FONTS; i++)
        glDeleteLists(m_fontBase[i], 96);
}

void Display::Initialize()
{
    m_viewX = 0.0f;
    m_viewY = 0.0f;
    m_viewZ = 0.0f;

    m_angleX = 0.0f;
    m_angleY = 0.0f;
    m_angleZ = 0.0f;

    m_targetX = 0.0f;
    m_targetY = 0.0f;
    m_targetZ = 0.0f;

    m_tarangleX = 0.0f;
    m_tarangleY = 0.0f;
    m_tarangleZ = 0.0f;

    m_targetmodel = NULL;

    m_is2D = false;
}

void Display::InitFont(uint8 font)
{
    if (font >= MAX_FONTS)
        return;

    float cx;
    float cy;

    GLint charsize = 16;
    if (font == FONT_ONE)
        charsize = 64;

    // V tomto pripade charsperline = charspercolumn
    uint32 charsperline = 16;
    float charoffset = 1.0f/charsperline;
    float spacing = 10.0f;

    if (font == FONT_ONE)
        spacing = 36.0f;

    m_fontBase[font] = glGenLists(256);

    // Textura s fontem
    if (!BindTexture(fontTextures[font]))
    {
        // Pockame na jeji nacteni
        while (sStorage->Textures[fontTextures[font]] == NULL)
            boost::this_thread::yield();
    }

    for (uint16 loop = 0; loop < 256; loop++)
    {
        cx = float(loop%charsperline)/float(charsperline);
        cy = float(loop/charsperline)/float(charsperline)+charoffset;

        glNewList(m_fontBase[font]+loop,GL_COMPILE);
            glBegin(GL_QUADS);
                glTexCoord2f(cx,           cy-charoffset); glVertex2i(0,       0);
                glTexCoord2f(cx+charoffset,cy-charoffset); glVertex2i(charsize,0);
                glTexCoord2f(cx+charoffset,cy);            glVertex2i(charsize,charsize);
                glTexCoord2f(cx,           cy);            glVertex2i(0,       charsize);
            glEnd();
            glTranslated(spacing,0,0);
        glEndList();
    }

    m_fontLoaded[font] = true;
}

//Vykresleni textu
void Display::PrintText(uint8 font, uint32 left, uint32 top, float scale, uint8 flags, uint32 color, const char *fmt, ...)
{
    if (font >= MAX_FONTS)
        return;

    if (!m_fontLoaded[font])
        InitFont(font);

    glEnable(GL_BLEND);

    // Nabindujeme texturu s fontem
    if (!BindTexture(fontTextures[font]))
    {
        // A pockame na jeji nacteni (pokud neni)
        while (sStorage->Textures[fontTextures[font]] == NULL)
            boost::this_thread::yield();
    }

    char text[512];
    va_list ap;

    if (fmt == NULL)
        return;

    va_start(ap, fmt);
      vsprintf(text, fmt, ap);
    va_end(ap);

    // Ted brat v potaz flagy
    if (flags & TEXT_FLAG_CENTERED_X)
    {
        float charsize = 10.0f;
        if (font == FONT_ONE)
            charsize = 36.0f;

        left = uint32(WIDTHPCT*50 - (charsize*scale)*(float(strlen(text)) / 2.0f));
    }
    if (flags & TEXT_FLAG_CENTERED_Y)
    {
        float charsize = 16.0f;
        if (font == FONT_ONE)
            charsize = 64.0f;

        top = uint32(HEIGHTPCT*50 - (charsize*scale*0.5f));
    }

    // Pokud jsme ve 3D rezimu
    bool in3D = !IsIn2DMode();

    // Prepneme do 2D
    if (in3D)
        Setup2DMode();

    glLoadIdentity();
    glTranslated(left,top,0);
    glColor4ub(CRED(color), CGREEN(color), CBLUE(color), 255-CALPHA(color));
    glScalef(scale, scale, scale);
    glListBase(m_fontBase[font]-32);
    glCallLists(strlen(text),GL_UNSIGNED_BYTE,text);
    glScalef(1.0f/scale, 1.0f/scale, 1.0f/scale);

    glLoadIdentity();

    glColor4ub(255, 255, 255, 255);

    // A po vykresleni se vratime zpet do puvodniho modu pokud je to nutne
    if (in3D)
        Setup3DMode();
}

bool Display::BindTexture(uint32 textureId)
{
    // Pokud jiz je aktualne nabindovana, neni treba nic delat
    if (textureId == m_boundTexture)
        return true;

    // Pokud neni nactena, nacteme ji
    if (sStorage->Textures[textureId] == NULL)
    {
        glDisable(GL_TEXTURE_2D);

        if (sLoader->IsCurrentlyLoaded(LOAD_TEXTURE, textureId))
            return false;

        sLoader->RequestLoad(LOAD_TEXTURE, textureId);
        return false;
    }

    if (!glIsEnabled(GL_TEXTURE_2D))
        glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, *(sStorage->Textures[textureId]));

    m_boundTexture = textureId;

    return true;
}

void Display::Update(const uint32 diff)
{
    if (m_is2D)
        Setup3DMode();

    sAnimator->Update();
    sParticleEmitterMgr->Update();

    DrawModels();
    DrawBillboards();
}

ModelDisplayListRecord* Display::DrawModel(uint32 modelId, float x, float y, float z, ModelAnimType Animation, float scale, float rotate, bool genGLDisplayList, bool animReverse, uint32 startFrame, uint32 frameSkipSpeed, AnimRestriction animRest, bool GLDisplayListOnly)
{
    ModelDisplayListRecord* pNew = new ModelDisplayListRecord;
    assert(pNew != NULL);

    pNew->modelId = modelId;
    pNew->x = x;
    pNew->y = y;
    pNew->z = z;
    if (Animation != ANIM_NONE)
    {
        pNew->AnimTicket = sAnimator->GetModelAnimTicket(modelId, Animation);
        sAnimator->ChangeModelAnim(pNew->AnimTicket, Animation, startFrame, frameSkipSpeed, (animReverse ? ANIM_FLAG_BOTHWAY : 0));
    }
    else
        pNew->AnimTicket = 0;

    pNew->CustomFrame = 0;

    pNew->animRestriction = animRest;

    pNew->remove = false;
    pNew->scale = scale;
    pNew->rotate = rotate;

    // Pokud potrebujeme vygenerovat displaylist, musime nacist i model a take pockat na jeho uplne nacteni
    if (sStorage->Models[modelId] == NULL)
    {
        if (!sLoader->IsCurrentlyLoaded(LOAD_MODEL, modelId))
            sLoader->RequestLoad(LOAD_MODEL, modelId);
    }

    if (!GLDisplayListOnly)
        ModelDisplayList.push_back(pNew);

    return pNew;
}

bool Display::RemoveRecordFromDisplayList(ModelDisplayListRecord* target)
{
    for (std::list<ModelDisplayListRecord*>::iterator itr = ModelDisplayList.begin(); itr != ModelDisplayList.end(); ++itr)
    {
        if (!itr._Has_container())
            continue;

        if ((*itr) == target)
        {
            (*itr)->remove = true;
            return true;
        }
    }

    return false;
}

bool Display::RemoveRecordFromDisplayList(BillboardDisplayListRecord* target)
{
    for (std::list<BillboardDisplayListRecord*>::iterator itr = BillboardDisplayList.begin(); itr != BillboardDisplayList.end(); ++itr)
    {
        if (!itr._Has_container())
            continue;

        if ((*itr) == target)
        {
            (*itr)->remove = true;
            return true;
        }
    }

    return false;
}

void ModelDisplayListRecord::AddFeature(ModelFeatureType type, float offset_x, float offset_y, float offset_z, void *feature)
{
    if (!feature || type >= MF_TYPE_MAX)
        return;

    ModelFeature* ft = new ModelFeature;
    ft->type = type;
    ft->feature = feature;
    ft->offset_x = offset_x;
    ft->offset_y = offset_y;
    ft->offset_z = offset_z;

    features.push_back(ft);
}

void ModelDisplayListRecord::ClearFeatures()
{
    if (features.empty())
        return;

    for (FeatureList::iterator itr = features.begin(); itr != features.end(); ++itr)
    {
        switch ((*itr)->type)
        {
            case MF_TYPE_MODEL:
                sDisplay->RemoveRecordFromDisplayList((*itr)->ToModel());
                break;
            case MF_TYPE_BILLBOARD:
                sDisplay->RemoveRecordFromDisplayList((*itr)->ToBillboard());
                break;
            case MF_TYPE_EMITTER:
                sParticleEmitterMgr->RemoveEmitter((*itr)->ToEmitter());
                break;
            default:
                break;
        }
    }

    features.clear();
}

void Display::DrawModels()
{
    float x,y,z;
    ModelDisplayListRecord* temp = NULL;

    for (std::list<ModelDisplayListRecord*>::iterator itr = ModelDisplayList.begin(); itr != ModelDisplayList.end();)
    {
        temp = *itr;

        if (!temp)
        {
            itr = ModelDisplayList.erase(itr);
            continue;
        }

        // Pokud ho jeste nenacetlo nacitaci vlakno, musime cekat..
        if (sStorage->Models[temp->modelId] == NULL)
        {
            if (!sLoader->IsCurrentlyLoaded(LOAD_MODEL, temp->modelId))
                sLoader->RequestLoad(LOAD_MODEL, temp->modelId);

            continue;
        }

        t3DModel* pModel = sStorage->Models[temp->modelId];

        if (temp->remove)
        {
            temp->ClearFeatures();

            if (temp->AnimTicket)
                sAnimator->DestroyAnimTicket(temp->AnimTicket);
            if (temp)
                delete temp;
            itr = ModelDisplayList.erase(itr);
            continue;
        }

        x = temp->x;
        y = temp->y;
        z = temp->z;

        sCustomAnimator->AnimateModelFeatures(temp);

        //if (pythagoras_c(fabs(fabs(x)-fabs(view_x)),fabs(fabs(z)-fabs(view_z))) > 2.0f)
        //{
        //    ++itr;
        //    continue;
        //}

        glLoadIdentity();

        glRotatef(m_angleX,1.0f,0.0f,0.0f);
        glRotatef(m_angleY,0.0f,1.0f,0.0f);
        glRotatef(m_angleZ,0.0f,0.0f,1.0f);

        glTranslatef(m_viewX, m_viewY, m_viewZ);
        glTranslatef(x,y,z);

        glRotatef(temp->rotate,0.0f,1.0f,0.0f);

        glScalef(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE);

        // Pokud mame vygenerovany GL display list pro dany model
        if (sStorage->Models[temp->modelId]->displayListSize != 0)
        {
            glScalef(temp->scale, temp->scale, temp->scale);
            // Vykreslime to pomoci display listu
            glCallList(sStorage->Models[temp->modelId]->displayList + sAnimator->GetActualFrame(temp->AnimTicket));
            // A nemusime se dale o nic starat
            ++itr;
            continue;
        }

        for (int i = 0; i < pModel->numOfObjects; i++)
        {
            if (pModel->pObject.size() <= 0)
                break;

            t3DObject *pObject = &pModel->pObject[i];

            glPushMatrix();
            AnimateModelObject(pObject, temp);

            if (pObject->bHasTexture)
            {
                glEnable(GL_TEXTURE_2D);
                glColor3ub(255, 255, 255);
                glBindTexture(GL_TEXTURE_2D, pModel->pMaterials[pObject->materialID].texureId);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
            else
            {
                glDisable(GL_TEXTURE_2D);
                if (pModel->pMaterials.size() && pObject->materialID >= 0)
                {
                    BYTE *pColor = pModel->pMaterials[pObject->materialID].color;
                    glColor3ub(pColor[0], pColor[1], pColor[2]);
                }
            }

            glBegin(GL_TRIANGLES);

            for (int j = 0; j < pObject->numOfFaces; j++)
            {
                for (int whichVertex = 0; whichVertex < 3; whichVertex++)
                {
                    int index = pObject->pFaces[j].vertIndex[whichVertex];

                    if (pObject->pNormals)
                    {
                        CVector3* pNormal = &pObject->pNormals[index];
                        glNormal3f(pNormal->x, pNormal->y, pNormal->z);
                    }

                    if (pObject->bHasTexture && pObject->pTexVerts != NULL)
                    {
                        CVector2* pTexVert = &pObject->pTexVerts[index];
                        glTexCoord2f(pTexVert->x, pTexVert->y);
                    }

                    if (pObject->pVerts)
                    {
                        CVector3* pVert = &pObject->pVerts[index];
                        glVertex3f(pVert->x, pVert->y, pVert->z);
                    }
                }
            }

            glEnd();

            glPopMatrix();
        }
        ++itr;
    }

    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glColor3ub(255, 255, 255);
}

void Display::FlushModelDisplayList()
{
    for (std::list<ModelDisplayListRecord*>::iterator itr = ModelDisplayList.begin(); itr != ModelDisplayList.end(); ++itr)
        (*itr)->remove = true;
}

void Display::AnimateModelObject(t3DObject *object, ModelDisplayListRecord* pData)
{
    uint32 frame = sAnimator->GetActualFrame(pData->AnimTicket);

    // Vlastni animace (nespecifikovana v souboru s modelem, treba skeletalni)
    if (sCustomAnimator->HaveModelCustomAnim(pData->modelId))
    {
        sCustomAnimator->AnimateModelObjectByFrame(object, pData->modelId, frame);
        return;
    }

    CVector3 vPosition = object->vPosition[frame];
    vPosition.multiply(pData->scale);
    glTranslatef(vPosition.x, vPosition.y, vPosition.z);
    CVector3 vScale = object->vScale[frame];
    vScale.multiply(pData->scale);
    glScalef(vScale.x, vScale.y, vScale.z);

    for (uint32 i = 1; i <= frame; i++)
    {
        CVector3 vRotation = object->vRotation[i];
        float rotDegree = object->vRotDegree[i];

        if(rotDegree)
            glRotatef(rotDegree, vRotation.x, vRotation.y, vRotation.z);
    }
}

void Display::AnimateModelObjectByFrame(t3DModel* model, t3DObject* object, uint32 modelId, uint32 frame)
{
    if (model && sCustomAnimator->HaveModelCustomAnim(modelId))
        sCustomAnimator->AnimateModelObjectByFrame(object, model, modelId, frame);
    else
        AnimateModelObjectByFrame(object, modelId, frame);
}

void Display::AnimateModelObjectByFrame(t3DObject* object, uint32 modelId, uint32 frame)
{
    // Vlastni animace (nespecifikovana v souboru s modelem, treba skeletalni)
    if (sCustomAnimator->HaveModelCustomAnim(modelId))
    {
        sCustomAnimator->AnimateModelObjectByFrame(object, modelId, frame);
        return;
    }

    CVector3 vPosition = object->vPosition[frame];
    glTranslatef(vPosition.x, vPosition.y, vPosition.z);
    CVector3 vScale = object->vScale[frame];
    glScalef(vScale.x, vScale.y, vScale.z);

    for (uint32 i = 1; i <= frame; i++)
    {
        CVector3 vRotation = object->vRotation[i];
        float rotDegree = object->vRotDegree[i];

        if(rotDegree)
            glRotatef(rotDegree, vRotation.x, vRotation.y, vRotation.z);
    }
}

BillboardDisplayListRecord* Display::DrawBillboard(uint32 textureId, float x, float y, float z, uint32 Animation, uint32 animFrameSpeed, float scale_x, float scale_y, bool billboard_x, bool billboard_y, bool genGLDisplayList, AnimRestriction animRest)
{
    BillboardDisplayListRecord* pNew = new BillboardDisplayListRecord;
    assert(pNew != NULL);

    pNew->textureId = textureId;
    pNew->x = x;
    pNew->y = y;
    pNew->z = z;
    if (Animation != ANIM_NONE)
        pNew->AnimTicket = sAnimator->GetTextureAnimTicket(textureId, Animation, animFrameSpeed);
    else
        pNew->AnimTicket = 0;

    pNew->animRestriction = animRest;

    pNew->remove = false;
    pNew->scale_x = scale_x;
    pNew->scale_y = scale_y;

    pNew->billboard_x = billboard_x;
    pNew->billboard_y = billboard_y;

    if (genGLDisplayList)
    {
        pNew->displayList = glGenLists(1);
        pNew->displayListSize = 1;

        glNewList(pNew->displayList,GL_COMPILE);
            glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(0, 0             , (-pNew->scale_x/2));
                glTexCoord2f(0.0f, 1.0f); glVertex3f(0, 0             , ( pNew->scale_x/2));
                glTexCoord2f(1.0f, 0.0f); glVertex3f(0, pNew->scale_y , (-pNew->scale_x/2));
                glTexCoord2f(0.0f, 0.0f); glVertex3f(0, pNew->scale_y , ( pNew->scale_x/2));
            glEnd();
        glEndList();
    }

    BillboardDisplayList.push_front(pNew);

    if (sStorage->Textures[textureId] == NULL)
    {
        if (!sLoader->IsCurrentlyLoaded(LOAD_TEXTURE, textureId))
            sLoader->RequestLoad(LOAD_TEXTURE, textureId);
    }

    return pNew;
}

// Vytvoreni billboard display list zaznamu bez jeho zapsani do display listu
BillboardDisplayListRecord* BillboardDisplayListRecord::Create(uint32 textureId, float x, float y, float z,
                                                               float scale_x, float scale_y,
                                                               bool billboard_x, bool billboard_y)
{
    BillboardDisplayListRecord* pNew = new BillboardDisplayListRecord;
    assert(pNew != NULL);

    pNew->textureId = textureId;
    pNew->x = x;
    pNew->y = y;
    pNew->z = z;

    pNew->AnimTicket = 0;

    pNew->remove = false;
    pNew->scale_x = scale_x;
    pNew->scale_y = scale_y;

    pNew->billboard_x = billboard_x;
    pNew->billboard_y = billboard_y;

    return pNew;
}

// Funkce pro serazeni zaznamu od nejvzdalenejsiho po nejblizsi kvuli vykreslovani
bool BubbleSortDistance(BillboardDisplayListRecord* first, BillboardDisplayListRecord* second)
{
    // Ve vypoctu musime zminusovat hracovo souradnice - vzdycky se pohybujeme s vykreslovanim v zaporne poloose, kdezto souradnice
    // realnych zaznamu se pohybuji v opacne poloose od tech hracskych

    // Pokud je vzdalenost daneho zaznamu od bodu pohledu vetsi nez u druheho, nechame ho vykreslit driv
    // true = "ano, posunout navrch"
    if (sqrt(pow(first->x-(-sDisplay->GetViewX()),2)+pow(first->y-(-sDisplay->GetViewY()),2)+pow(first->z-(-sDisplay->GetViewZ()),2))
        > sqrt(pow(second->x-(-sDisplay->GetViewX()),2)+pow(second->y-(-sDisplay->GetViewY()),2)+pow(second->z-(-sDisplay->GetViewZ()),2)))
        return true;

    return false;
}

void Display::DrawBillboards()
{
    BillboardDisplayListRecord* temp = NULL;

    glDepthMask(GL_FALSE);

    bool lightingEnable = (glIsEnabled(GL_LIGHTING) == GL_TRUE);
    glDisable(GL_LIGHTING);

    if (!BillboardDisplayList.empty())
        BillboardDisplayList.sort(BubbleSortDistance);

    // Pruhledne objekty potrebuji mit zapnuty mod pro blending a mod one minus src alpha pro
    // spravne vykresleni pruhlednosti
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Nechat depth test pro ted zapnuty, zpusobuje nepruhlednost
    // vykreslenych spritu vuci jinym spritum
    //glDisable(GL_DEPTH_TEST);

    glColor3ub(255, 255, 255);

    for (std::list<BillboardDisplayListRecord*>::iterator itr = BillboardDisplayList.begin(); itr != BillboardDisplayList.end();)
    {
        temp = (*itr);

        if (!temp)
        {
            itr = BillboardDisplayList.erase(itr);
            continue;
        }

        if (temp->remove)
        {
            if (temp->displayList)
                glDeleteLists(temp->displayList, temp->displayListSize);
            if (temp->AnimTicket)
                sAnimator->DestroyAnimTicket(temp->AnimTicket);
            if (temp)
                delete temp;
            itr = BillboardDisplayList.erase(itr);
            continue;
        }

        // Zabezpeceni proti vykreslovani billboardu, ktere jsou od nas vzdalene vice jak 14 jednotek
        // - Zvysuje vykon
        if (pythagoras_c(fabs(temp->x-fabs(m_targetX)), fabs(temp->z-fabs(m_targetZ))) > 14.0f)
        {
            ++itr;
            continue;
        }

        glLoadIdentity();

        AdjustViewToTarget();

        if (temp->AnimTicket)
            BindTexture(sAnimator->GetActualTexture(temp->AnimTicket));
        else
            BindTexture(temp->textureId);

        glTranslatef(temp->x, temp->y, temp->z);
        if (temp->billboard_y)
            glRotatef(90.0f-m_angleY,  0.0f, 1.0f, 0.0f);
        if (temp->billboard_x)
            glRotatef(m_angleX, 0.0f, 0.0f, -1.0f);

        if (temp->displayList > 0)
        {
            glCallList(temp->displayList);
        }
        else
        {
            glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(0, 0             , (-temp->scale_x/2));
                glTexCoord2f(0.0f, 1.0f); glVertex3f(0, 0             , ( temp->scale_x/2));
                glTexCoord2f(1.0f, 0.0f); glVertex3f(0, temp->scale_y , (-temp->scale_x/2));
                glTexCoord2f(0.0f, 0.0f); glVertex3f(0, temp->scale_y , ( temp->scale_x/2));
            glEnd();
        }

        ++itr;
    }
    // Nezapomeneme vypnout blending, jen jako slusnacci
    glDisable(GL_BLEND);

    // Netreba, viz. komentar vyse
    //glEnable(GL_DEPTH_TEST);

    glDepthMask(GL_TRUE);

    if (lightingEnable)
        glEnable(GL_LIGHTING);

    glLoadIdentity();

    glRotatef(m_angleX, 1.0f,  0.0f, 0.0f);
    glRotatef(m_angleY, 0.0f, -1.0f, 0.0f);
    glTranslatef(m_viewX, m_viewY, m_viewZ);
}

void Display::FlushBillboardDisplayList()
{
    for (std::list<BillboardDisplayListRecord*>::iterator itr = BillboardDisplayList.begin(); itr != BillboardDisplayList.end(); ++itr)
        (*itr)->remove = true;
}

void Display::EnableRestrictedAnimations(AnimRestriction animRes)
{
    for (std::list<ModelDisplayListRecord*>::iterator itr = ModelDisplayList.begin(); itr != ModelDisplayList.end(); ++itr)
        if ((*itr)->animRestriction == animRes)
            sAnimator->EnableAnimation((*itr)->AnimTicket);

    for (std::list<BillboardDisplayListRecord*>::iterator itr = BillboardDisplayList.begin(); itr != BillboardDisplayList.end(); ++itr)
        if ((*itr)->animRestriction == animRes)
            sAnimator->EnableAnimation((*itr)->AnimTicket);
}

void Display::DisableRestrictedAnimations(AnimRestriction animRes)
{
    for (std::list<ModelDisplayListRecord*>::iterator itr = ModelDisplayList.begin(); itr != ModelDisplayList.end(); ++itr)
        if ((*itr)->animRestriction == animRes)
            sAnimator->DisableAnimation((*itr)->AnimTicket);

    for (std::list<BillboardDisplayListRecord*>::iterator itr = BillboardDisplayList.begin(); itr != BillboardDisplayList.end(); ++itr)
        if ((*itr)->animRestriction == animRes)
            sAnimator->DisableAnimation((*itr)->AnimTicket);
}

void Display::Setup2DMode()
{
    // Priprava a prechod do 2D projekce
    int vPort[4];
    glGetIntegerv(GL_VIEWPORT, vPort);

    // Bez svetel ve 2D modu
    m_cachedLighting = (glIsEnabled(GL_LIGHTING) == GL_TRUE);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(vPort[0], vPort[0]+vPort[2], vPort[1]+vPort[3], vPort[1]);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.375f, 0.375f, 0.0f);

    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);

    m_is2D = true;
}

void Display::Setup3DMode()
{
    // Prechod zpatky do 3D, vraceni matrixu do puvodnich parametru
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);

    if (m_cachedLighting)
        glEnable(GL_LIGHTING);

    m_is2D = false;
}

void Display::Draw2D(uint32 textureId, float left, float top, float width, float height)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    if (!BindTexture(textureId))
        return;

    glBegin(GL_QUADS);
      glColor3ub(255, 255, 255);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(left+width, top+0);
        glTexCoord2f(1.0f, 1.0f); glVertex2d(left+width, top+height);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(left+0, top+height);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(left+0, top+0);
    glEnd();
}

void Display::DrawMap()
{
    Map* pMap = (Map*)sMapManager->GetMap();
    if (!pMap || pMap->field.size() < 1 || pMap->field[0].size() < 1)
        return;

    // Pracovni promenne pro vykreslovani
    GLfloat x,y,z;
    Map::DynamicCellSet::const_iterator itr;

    glLoadIdentity();
    glRotatef(m_angleX,1.0f,0.0f,0.0f);
    glRotatef(m_angleY,0.0f,1.0f,0.0f);
    glRotatef(m_angleZ,0.0f,0.0f,1.0f);
    glTranslatef(m_viewX, m_viewY, m_viewZ);

    // Vykresleni skyboxu

    /*
    Skybox data struktura:
        id
        box_textures[6]
            0 = dolni
            1 = horni
            2 = zadni
            3 = predni
            4 = vlevo
            5 = vpravo
    */
    uint32 m_skybox = pMap->m_skybox;
    GLfloat skyboxSize = 50.0f;

    // Predni cast
    sDisplay->BindTexture(sStorage->SkyboxData[m_skybox].box_textures[3]);
    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 0.0f, 1.0f);
        glTexCoord2f(1, 1); glVertex3f(  skyboxSize, -skyboxSize, -skyboxSize );
        glTexCoord2f(0, 1); glVertex3f( -skyboxSize, -skyboxSize, -skyboxSize );
        glTexCoord2f(0, 0); glVertex3f( -skyboxSize,  skyboxSize, -skyboxSize );
        glTexCoord2f(1, 0); glVertex3f(  skyboxSize,  skyboxSize, -skyboxSize );
    glEnd();

    // Cast vlevo
    sDisplay->BindTexture(sStorage->SkyboxData[m_skybox].box_textures[4]);
    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(1, 1); glVertex3f(  skyboxSize, -skyboxSize,  skyboxSize );
        glTexCoord2f(0, 1); glVertex3f(  skyboxSize, -skyboxSize, -skyboxSize );
        glTexCoord2f(0, 0); glVertex3f(  skyboxSize,  skyboxSize, -skyboxSize );
        glTexCoord2f(1, 0); glVertex3f(  skyboxSize,  skyboxSize,  skyboxSize );
    glEnd();

    // Zadni cast
    sDisplay->BindTexture(sStorage->SkyboxData[m_skybox].box_textures[2]);
    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 0.0f,-1.0f);
        glTexCoord2f(1, 1); glVertex3f( -skyboxSize, -skyboxSize,  skyboxSize );
        glTexCoord2f(0, 1); glVertex3f(  skyboxSize, -skyboxSize,  skyboxSize );
        glTexCoord2f(0, 0); glVertex3f(  skyboxSize,  skyboxSize,  skyboxSize );
        glTexCoord2f(1, 0); glVertex3f( -skyboxSize,  skyboxSize,  skyboxSize );
    glEnd();

    // Cast vpravo
    sDisplay->BindTexture(sStorage->SkyboxData[m_skybox].box_textures[5]);
    glBegin(GL_QUADS);
        glNormal3f( 1.0f, 0.0f, 0.0f);
        glTexCoord2f(1, 1); glVertex3f( -skyboxSize, -skyboxSize, -skyboxSize );
        glTexCoord2f(0, 1); glVertex3f( -skyboxSize, -skyboxSize,  skyboxSize );
        glTexCoord2f(0, 0); glVertex3f( -skyboxSize,  skyboxSize,  skyboxSize );
        glTexCoord2f(1, 0); glVertex3f( -skyboxSize,  skyboxSize, -skyboxSize );
    glEnd();

    // Strop
    sDisplay->BindTexture(sStorage->SkyboxData[m_skybox].box_textures[1]);
    glBegin(GL_QUADS);
        glNormal3f( 0.0f, 1.0f, 0.0f);
        glTexCoord2f(0, 1); glVertex3f( -skyboxSize,  skyboxSize, -skyboxSize );
        glTexCoord2f(0, 0); glVertex3f( -skyboxSize,  skyboxSize,  skyboxSize );
        glTexCoord2f(1, 0); glVertex3f(  skyboxSize,  skyboxSize,  skyboxSize );
        glTexCoord2f(1, 1); glVertex3f(  skyboxSize,  skyboxSize, -skyboxSize );
    glEnd();

    // Spodni cast
    sDisplay->BindTexture(sStorage->SkyboxData[m_skybox].box_textures[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -skyboxSize, -skyboxSize, -skyboxSize );
        glTexCoord2f(0, 1); glVertex3f( -skyboxSize, -skyboxSize,  skyboxSize );
        glTexCoord2f(1, 1); glVertex3f(  skyboxSize, -skyboxSize,  skyboxSize );
        glTexCoord2f(1, 0); glVertex3f(  skyboxSize, -skyboxSize, -skyboxSize );
    glEnd();

    // Konec vykresleni skyboxu

    Map::Cell* tempField = NULL;

    for (uint32 w = 0; w < pMap->field.size(); w++)
    {
        for (uint32 h = 0; h < pMap->field[0].size(); h++)
        {
            x = w;
            y = 0;
            z = h;

            tempField = &pMap->field[w][h];

            BindTexture(tempField->texture);

            // Nejdrive staticke prvky
            if (tempField->type == TYPE_GROUND || tempField->type == TYPE_STARTLOC)
            {
                glBegin(GL_POLYGON);
                    glNormal3f(0.0f,-1.0f, 0.0f);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(x-1, y, z-1);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(x  , y, z-1);
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(x  , y, z  );
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(x-1, y, z  );
                glEnd();
            }
            else if (tempField->type == TYPE_SOLID_BOX)
            {
                Storage::SolidBoxDataMap::const_iterator itr = sStorage->SolidBoxProp.find(tempField->texture);

                if (itr != sStorage->SolidBoxProp.end())
                {
                    if (itr->second.model_id == 0)
                    {
                        BindTexture(itr->second.texture_top);
                        glBegin(GL_QUADS);
                            glNormal3f(0.0f,-1.0f, 0.0f);
                            // horni
                            glNormal3f( 0.0f, 1.0f, 0.0f);
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(x-1, y+SOLID_BOX_HEIGHT, z-1);
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(x  , y+SOLID_BOX_HEIGHT, z-1);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(x  , y+SOLID_BOX_HEIGHT, z  );
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(x-1, y+SOLID_BOX_HEIGHT, z  );
                        glEnd();
                        BindTexture(itr->second.texture_sides[0]);
                        glBegin(GL_QUADS);
                            // levobok
                            glNormal3f(-1.0f, 0.0f, 0.0f);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(x-1, y+SOLID_BOX_HEIGHT, z-1);
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(x-1, y                 , z-1);
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(x-1, y                 , z  );
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(x-1, y+SOLID_BOX_HEIGHT, z  );
                        glEnd();
                        BindTexture(itr->second.texture_sides[1]);
                        glBegin(GL_QUADS);
                            // pravobok
                            glNormal3f( 1.0f, 0.0f, 0.0f);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(x  , y+SOLID_BOX_HEIGHT, z  );
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(x  , y                 , z  );
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(x  , y                 , z-1);
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(x  , y+SOLID_BOX_HEIGHT, z-1);
                        glEnd();
                        BindTexture(itr->second.texture_sides[2]);
                        glBegin(GL_QUADS);
                            // predni
                            glNormal3f( 0.0f, 0.0f, 1.0f);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(x-1, y+SOLID_BOX_HEIGHT, z  );
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(x-1, y                 , z  );
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(x  , y                 , z  );
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(x  , y+SOLID_BOX_HEIGHT, z  );
                        glEnd();
                        BindTexture(itr->second.texture_sides[3]);
                        glBegin(GL_QUADS);
                            // zadni
                            glNormal3f( 0.0f, 0.0f,-1.0f);
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(x-1, y+SOLID_BOX_HEIGHT, z-1);
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(x-1, y                 , z-1);
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(x  , y                 , z-1);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(x  , y+SOLID_BOX_HEIGHT, z-1);
                        glEnd();
                    }
                    else
                    {
                        BindTexture(itr->second.texture_top);
                        glBegin(GL_POLYGON);
                            glNormal3f(0.0f,-1.0f, 0.0f);
                            glTexCoord2f(1.0f, 1.0f); glVertex3f(x-1, y, z-1);
                            glTexCoord2f(0.0f, 1.0f); glVertex3f(x  , y, z-1);
                            glTexCoord2f(0.0f, 0.0f); glVertex3f(x  , y, z  );
                            glTexCoord2f(1.0f, 0.0f); glVertex3f(x-1, y, z  );
                        glEnd();

                        if (!tempField->pRec)
                            tempField->pRec = DrawModel(itr->second.model_id, w-0.5f, 0.0f, h-0.5f, ANIM_IDLE, 0.2f, 45.0f, true, false, 0, 0, ANIM_RESTRICTION_NONE);
                    }
                }
            }
        }
    }
}

void Display::SetTargetModel(ModelDisplayListRecord* pTarget)
{
    m_targetmodel = pTarget;
}

void Display::AdjustViewToTarget()
{
    if (m_targetmodel)
    {
        m_targetX = -m_targetmodel->x;
        m_targetY = -m_targetmodel->y;
        m_targetZ = -m_targetmodel->z;
        m_tarangleY = -m_targetmodel->rotate-270.0f;
    }

    // Odchylka horizontalniho uhlu - pouziva se treba pri pohybu hrace do stran
    if (m_deviateAngleY != 0.0f)
        m_tarangleY += m_deviateAngleY;

    if (m_targetmodel)
        m_angleY = 90.0f + m_tarangleY;

    if (m_targetmodel)
    {
        m_viewX = m_targetX + 1.8f*cos(PI*(m_tarangleY)/180.0f);
        m_viewY = -2.0f; // potrebuje upresnit, vyska pohledu, pri FPS minimalni
        m_viewZ = m_targetZ + 1.8f*sin(PI*(m_tarangleY)/180.0f);

        m_angleX = 30.0f; // 0 pri FPS
    }

    glLoadIdentity();
    glRotatef(m_angleX,1.0f,0.0f,0.0f);
    glRotatef(m_angleY,0.0f,1.0f,0.0f);
    glRotatef(m_angleZ,0.0f,0.0f,1.0f);
    glTranslatef(m_viewX, m_viewY, m_viewZ);
}

uint16 Display::CheckCollision(float newx, float newy, float newz)
{
    const Map* pMap = sMapManager->GetMap();
    if (!pMap)
        return (AXIS_X | AXIS_Y | AXIS_Z);

    uint16 collision = 0;

    int32 ContentPosX = int(fabs(newx))+1;
    int32 ContentPosZ = int(fabs(newz))+1;
    int32 ContentPosXO = int(fabs(m_targetmodel ? m_targetX : m_viewX))+1;
    int32 ContentPosZO = int(fabs(m_targetmodel ? m_targetZ : m_viewZ))+1;

    if(ContentPosX+1 >= pMap->field.size())
        collision |= AXIS_X;
    if(ContentPosZ+1 >= pMap->field[0].size())
        collision |= AXIS_Z;

    if(ContentPosX <= 0)
        collision |= AXIS_X;
    if(ContentPosZ <= 0)
        collision |= AXIS_Z;

    if(newx < 0)
        collision |= AXIS_X;
    if(newz < 0)
        collision |= AXIS_Z;

    int32 lh_x = ContentPosX-1;
    int32 lh_z = ContentPosZ-1;
    int32 pd_x = ContentPosX+1;
    int32 pd_z = ContentPosZ+1;
    if (lh_x < 0)
        lh_x = 0;
    if (lh_z < 0)
        lh_z = 0;
    if (pd_x > pMap->field.size()-1)
        pd_x = pMap->field.size()-1;
    if (pd_z > pMap->field[0].size()-1)
        pd_z = pMap->field[0].size()-1;

    float ncx = fabs(newx);
    float ncz = fabs(newz);
    float ocx = fabs(m_targetmodel ? m_targetX : m_viewX);
    float ocz = fabs(m_targetmodel ? m_targetZ : m_viewZ);
    float conx = 0;
    float conz = 0;
    float sizex = 1;
    float sizez = 1;

    bool ignored = false;

    for (int32 i = lh_x; i <= pd_x; i++)
    {
        for (int32 j = lh_z; j <= pd_z; j++)
        {
            // Kolize s pevnymi objekty mapy
            if (pMap->field[i][j].type == TYPE_SOLID_BOX)
            {
                conx = (i-1);
                conz = (j-1);

                if (ncx < conx+sizex+COLLISION_RANGE && ncx > conx-COLLISION_RANGE &&
                    ncz < conz+sizez+COLLISION_RANGE && ncz > conz-COLLISION_RANGE)
                {
                    if (ncz != ocz)
                    {
                        if (ocz >= conz+sizez+COLLISION_RANGE || ocz <= conz-COLLISION_RANGE)
                            if (ocx < conx+sizex+COLLISION_RANGE || ocx > conx-COLLISION_RANGE)
                                collision |= AXIS_Z;
                    }
                    if (ncx != ocx)
                    {
                        if (ocx >= conx+sizex+COLLISION_RANGE || ocx <= conx-COLLISION_RANGE)
                            if (ocz < conz+sizez+COLLISION_RANGE || ocz > conz-COLLISION_RANGE)
                                collision |= AXIS_X;
                    }
                }
            }
            // Kolize s dynamickymi prvky mapy
            else if (pMap->dynfield[i][j].size() > 0)
            {
                for (uint32 k = 0; k < pMap->dynfield[i][j].size(); k++)
                {
                    switch (pMap->dynfield[i][j][k].type)
                    {
                        case DYNAMIC_TYPE_BOMB:
                        case DYNAMIC_TYPE_BOX:
                        {
                            /* Kod zde je prakticky duplikatem vyse vypsaneho. Je lepsi ho ale zduplikovat,
                               protoze pro dynamicke prvky musime iterovat zkrz cely list dynamickych prvku
                               na jedno pole. Lepsich par bajtu navic, nez nejaka prasarna.
                            */
                            conx = (i-1);
                            conz = (j-1);

                            if (ncx < conx+sizex+COLLISION_RANGE && ncx > conx-COLLISION_RANGE &&
                                ncz < conz+sizez+COLLISION_RANGE && ncz > conz-COLLISION_RANGE)
                            {
                                if (ncz != ocz)
                                {
                                    if (ocz >= conz+sizez+COLLISION_RANGE || ocz <= conz-COLLISION_RANGE)
                                        if (ocx < conx+sizex+COLLISION_RANGE || ocx > conx-COLLISION_RANGE)
                                        {
                                            if (m_ignoreTargetCollision == pMap->dynfield[i][j][k].type)
                                            {
                                                ignored = true;
                                                break;
                                            }
                                            collision |= AXIS_Z;
                                        }
                                }
                                if (ncx != ocx)
                                {
                                    if (ocx >= conx+sizex+COLLISION_RANGE || ocx <= conx-COLLISION_RANGE)
                                        if (ocz < conz+sizez+COLLISION_RANGE || ocz > conz-COLLISION_RANGE)
                                        {
                                            if (m_ignoreTargetCollision == pMap->dynfield[i][j][k].type)
                                            {
                                                ignored = true;
                                                break;
                                            }
                                            collision |= AXIS_X;
                                        }
                                }
                            }
                        }
                        break;
                    }

                    if (!ignored && m_ignoreTargetCollision > 0)
                        m_ignoreTargetCollision = 0;
                }
            }
        }
    }

    return collision;
}
