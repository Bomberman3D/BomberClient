// Original code by DigiBen (digiben@gametutorials.com)
#include <Global.h>
#include <ModelLoader.h>
#include <Helpers.h>
#include <Storage.h>

// Global
int gBuffer[50000] = {0};

Loaders::t3DSLoader::t3DSLoader()
{
    m_CurrentChunk = new tChunk;
    m_TempChunk = new tChunk;
}

Loaders::t3DSLoader::~t3DSLoader()
{
    delete m_CurrentChunk;
    delete m_TempChunk;
}

bool Loaders::t3DSLoader::Import3DS(t3DModel *pModel, char *strFileName)
{
    char strMessage[255] = {0};

    m_FilePointer = fopen(strFileName, "rb");

    if(!m_FilePointer)
    {
        sprintf(strMessage, "Unable to find the file: %s!", strFileName);
        MessageBox(NULL, strMessage, "Error", MB_OK);
        return false;
    }

    ReadChunk(m_CurrentChunk);

    if (m_CurrentChunk->ID != PRIMARY)
    {
        sprintf(strMessage, "Unable to load PRIMARY chuck from file: %s!", strFileName);
        MessageBox(NULL, strMessage, "Error", MB_OK);
        return false;
    }

    ProcessNextChunk(pModel, m_CurrentChunk);

    pModel->currentScale = 1.0f;

    ResizeObjects(pModel);

    ComputeNormals(pModel);

    CleanUp();

    return true;
}

void Loaders::t3DSLoader::ResizeObjects(t3DModel* pModel)
{
    if (pModel->numOfObjects <= 0)
        return;

    float nx, ny, nz;

    for (int index = 0; index < pModel->numOfObjects; index++)
    {
        nx = pModel->pObject[index].vScale[0].x - 1.0f;
        ny = pModel->pObject[index].vScale[0].y - 1.0f;
        nz = pModel->pObject[index].vScale[0].z - 1.0f;

        for (int i = 0; i <= pModel->numberOfFrames; ++i)
        {
            if (pModel->pObject[index].vScale.size() <= i)
            {
                pModel->pObject[index].vScale.resize(pModel->pObject[index].vScale.size()+1);
                pModel->pObject[index].vScale[i].x = 1;
                pModel->pObject[index].vScale[i].y = 1;
                pModel->pObject[index].vScale[i].z = 1;
            }

            if (pModel->pObject[index].vPosition.size() <= i)
            {
                pModel->pObject[index].vPosition.resize(pModel->pObject[index].vPosition.size()+1);
                pModel->pObject[index].vPosition[i].x = 0.0f;
                pModel->pObject[index].vPosition[i].y = 0.0f;
                pModel->pObject[index].vPosition[i].z = 0.0f;
            }

            pModel->pObject[index].vScale[i].x -= nx;
            pModel->pObject[index].vScale[i].y -= ny;
            pModel->pObject[index].vScale[i].z -= nz;
        }
    }
}

void Loaders::t3DSLoader::CleanUp()
{
    fclose(m_FilePointer);
}

void Loaders::t3DSLoader::ProcessNextChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
    t3DObject newObject;
    tMaterialInfo newTexture = {0};
    int version = 0;

    m_CurrentChunk = new tChunk;

    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        ReadChunk(m_CurrentChunk);

        switch (m_CurrentChunk->ID)
        {
        case VERSION:
            m_CurrentChunk->bytesRead += fread(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);

            if (version > 0x03)
                MessageBox(NULL, "This 3DS file is over version 3 so it may load incorrectly", "Warning", MB_OK);
            break;
        case OBJECTINFO:
            ReadChunk(m_TempChunk);

            m_TempChunk->bytesRead += fread(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

            m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;

            ProcessNextChunk(pModel, m_CurrentChunk);
            break;
        case MATERIAL:
            pModel->numOfMaterials++;

            pModel->pMaterials.push_back(newTexture);

            ProcessNextMaterialChunk(pModel, m_CurrentChunk);
            break;
        case OBJECT:
            pModel->numOfObjects++;

            pModel->pObject.push_back(newObject);

            memset(&(pModel->pObject[pModel->numOfObjects - 1]), 0, sizeof(t3DObject));

            m_CurrentChunk->bytesRead += GetString(pModel->pObject[pModel->numOfObjects - 1].strName);

            ProcessNextObjectChunk(pModel, &(pModel->pObject[pModel->numOfObjects - 1]), m_CurrentChunk);
            break;
        case KEYFRAME:
            ProcessNextKeyFrameChunk(pModel, m_CurrentChunk);

            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        default:
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

void Loaders::t3DSLoader::ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
    m_CurrentChunk = new tChunk;

    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        ReadChunk(m_CurrentChunk);

        switch (m_CurrentChunk->ID)
        {
        case OBJECT_MESH:
            ProcessNextObjectChunk(pModel, pObject, m_CurrentChunk);
            break;
        case OBJECT_VERTICES:
            ReadVertices(pObject, m_CurrentChunk);
            break;
        case OBJECT_FACES:
            ReadVertexIndices(pObject, m_CurrentChunk);
            break;
        case OBJECT_MATERIAL:
            ReadObjectMaterial(pModel, pObject, m_CurrentChunk);
            break;
        case OBJECT_UV:
            ReadUVCoordinates(pObject, m_CurrentChunk);
            break;
        default:
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

void Loaders::t3DSLoader::ProcessNextMaterialChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
    m_CurrentChunk = new tChunk;

    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        ReadChunk(m_CurrentChunk);

        switch (pModel, m_CurrentChunk->ID)
        {
        case MATNAME:
            m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        case MATDIFFUSE:
            ReadColorChunk(&(pModel->pMaterials[pModel->numOfMaterials - 1]), m_CurrentChunk);
            break;
        case MATMAP:
            ProcessNextMaterialChunk(pModel, m_CurrentChunk);
            break;
        case MATMAPFILE:
            m_CurrentChunk->bytesRead += fread(pModel->pMaterials[pModel->numOfMaterials - 1].strFile, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        case MATSPECULAR:
            ReadColorChunkDest((uint8*)&(pModel->pMaterials[pModel->numOfMaterials - 1].specularcolor[0]), m_CurrentChunk);
            break;
        case MATSHININESS:
            ReadPercentageChunkDest((uint8*)&(pModel->pMaterials[pModel->numOfMaterials - 1].shininess.uShininess[0]), m_CurrentChunk);
            break;
        case MATSHININESSSTR:
            ReadPercentageChunkDest((uint8*)&(pModel->pMaterials[pModel->numOfMaterials - 1].shininess.uShininess[1]), m_CurrentChunk);
            break;
        case MATSHININESSSTR2:
            ReadPercentageChunkDest((uint8*)&(pModel->pMaterials[pModel->numOfMaterials - 1].shininess.uShininess[2]), m_CurrentChunk);
            break;
        default:
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

void Loaders::t3DSLoader::ProcessNextKeyFrameChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
    char strKeyFrameObject[50] = {0};
    float temp = 0.0f;

    m_CurrentChunk = new tChunk;

    while (pPreviousChunk->bytesRead < pPreviousChunk->length)
    {
        ReadChunk(m_CurrentChunk);

        switch (m_CurrentChunk->ID)
        {
        case KEYFRAME_MESH_INFO:
            ProcessNextKeyFrameChunk(pModel, m_CurrentChunk);
            break;
        case KEYFRAME_OBJECT_NAME:
            m_CurrentChunk->bytesRead += GetString(strKeyFrameObject);
            SetCurrentObject(pModel, strKeyFrameObject);

            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        case KEYFRAME_START_AND_END:
            m_CurrentChunk->bytesRead += fread(&(pModel->numberOfFrames), 1, 4, m_FilePointer);
            m_CurrentChunk->bytesRead += fread(&(pModel->numberOfFrames), 1, 4, m_FilePointer);
            break;
        case PIVOT:
            m_CurrentChunk->bytesRead +=
            fread(&(m_CurrentObject->vPivot), 1, sizeof(CVector3), m_FilePointer);

            temp = m_CurrentObject->vPivot.y;
            m_CurrentObject->vPivot.y = m_CurrentObject->vPivot.z;
            m_CurrentObject->vPivot.z = -temp;
            break;
        case POSITION_TRACK_TAG:
            ReadKeyFramePositions(pModel, m_CurrentChunk);
            break;
        case ROTATION_TRACK_TAG:
            ReadKeyFrameRotations(pModel, m_CurrentChunk);
            break;
        case SCALE_TRACK_TAG:
            ReadKeyFrameScales(pModel, m_CurrentChunk);
            break;
        default:
            m_CurrentChunk->bytesRead += fread(gBuffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
            break;
        }

        pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
    }

    delete m_CurrentChunk;
    m_CurrentChunk = pPreviousChunk;
}

void Loaders::t3DSLoader::ReadChunk(tChunk *pChunk)
{
    pChunk->bytesRead = fread(&pChunk->ID, 1, 2, m_FilePointer);

    pChunk->bytesRead += fread(&pChunk->length, 1, 4, m_FilePointer);
}

int Loaders::t3DSLoader::GetString(char *pBuffer)
{
    int index = 0;

    fread(pBuffer, 1, 1, m_FilePointer);

    while (*(pBuffer + index++) != 0) {

        fread(pBuffer + index, 1, 1, m_FilePointer);
    }

    return strlen(pBuffer) + 1;
}

void Loaders::t3DSLoader::ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk)
{
    ReadChunk(m_TempChunk);

    m_TempChunk->bytesRead += fread(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

    pChunk->bytesRead += m_TempChunk->bytesRead;
}

void Loaders::t3DSLoader::ReadColorChunkDest(uint8 *dest, tChunk *pChunk)
{
    ReadChunk(m_TempChunk);

    m_TempChunk->bytesRead += fread(dest, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

    pChunk->bytesRead += m_TempChunk->bytesRead;
}

void Loaders::t3DSLoader::ReadPercentageChunkDest(uint8 *dest, tChunk* pChunk)
{
    ReadChunk(m_TempChunk);

    m_TempChunk->bytesRead += fread(dest, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

    pChunk->bytesRead += m_TempChunk->bytesRead;
}

void Loaders::t3DSLoader::ReadVertexIndices(t3DObject *pObject, tChunk *pPreviousChunk)
{
    unsigned short index = 0;

    pPreviousChunk->bytesRead += fread(&pObject->numOfFaces, 1, 2, m_FilePointer);

    pObject->pFaces = new tFace [pObject->numOfFaces];
    memset(pObject->pFaces, 0, sizeof(tFace) * pObject->numOfFaces);

    for(int i = 0; i < pObject->numOfFaces; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            pPreviousChunk->bytesRead += fread(&index, 1, sizeof(index), m_FilePointer);

            if(j < 3)
                pObject->pFaces[i].vertIndex[j] = index;
        }
    }
}

void Loaders::t3DSLoader::ReadUVCoordinates(t3DObject *pObject, tChunk *pPreviousChunk)
{
    pPreviousChunk->bytesRead += fread(&pObject->numTexVertex, 1, 2, m_FilePointer);

    pObject->pTexVerts = new CVector2 [pObject->numTexVertex];

    pPreviousChunk->bytesRead += fread(pObject->pTexVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

void Loaders::t3DSLoader::ReadVertices(t3DObject *pObject, tChunk *pPreviousChunk)
{
    pPreviousChunk->bytesRead += fread(&(pObject->numOfVerts), 1, 2, m_FilePointer);

    pObject->pVerts = new CVector3 [pObject->numOfVerts];
    memset(pObject->pVerts, 0, sizeof(CVector3) * pObject->numOfVerts);

    pPreviousChunk->bytesRead += fread(pObject->pVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);

    for(int i = 0; i < pObject->numOfVerts; i++)
    {
        float fTempY = pObject->pVerts[i].y;

        pObject->pVerts[i].y = pObject->pVerts[i].z;

        pObject->pVerts[i].z = -fTempY;
    }
}

void Loaders::t3DSLoader::ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
    char strMaterial[255] = {0};

    pPreviousChunk->bytesRead += GetString(strMaterial);

    for(int i = 0; i < pModel->numOfMaterials; i++)
    {
        if(strcmp(strMaterial, pModel->pMaterials[i].strName) == 0)
        {
            pObject->materialID = i;

            if(strlen(pModel->pMaterials[i].strFile) > 0)
                pObject->bHasTexture = true;
            break;
        }
        else
            pObject->materialID = -1;
    }

    pPreviousChunk->bytesRead += fread(gBuffer, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

void Loaders::t3DSLoader::ReadKeyFramePositions(t3DModel *pModel, tChunk *pPreviousChunk)
{
    short frameNumber= 0, flags= 0, ignored= 0;
    long  lunknown= 0;
    int i;

    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);

    pPreviousChunk->bytesRead += fread(&(m_CurrentObject->positionFrames), 1, sizeof(short), m_FilePointer);

    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);

    for(i = 0; i <= (pModel->numberOfFrames + 1); i++)
    {
        m_CurrentObject->vPosition.push_back(CVector3());

        if(i < m_CurrentObject->positionFrames)
        {
            pPreviousChunk->bytesRead += fread(&frameNumber, 1, sizeof(short), m_FilePointer);

            pPreviousChunk->bytesRead += fread(&lunknown, 1, sizeof(long), m_FilePointer);

            pPreviousChunk->bytesRead +=
            fread(&(m_CurrentObject->vPosition[i]), 1, sizeof(CVector3), m_FilePointer);

            float temp = m_CurrentObject->vPosition[i].y;
            m_CurrentObject->vPosition[i].y = m_CurrentObject->vPosition[i].z;
            m_CurrentObject->vPosition[i].z = -temp;
        }
        else
            m_CurrentObject->vPosition[i] = m_CurrentObject->vPosition[m_CurrentObject->positionFrames - 1];
    }

    for(i = 0; i < m_CurrentObject->numOfVerts; i++)
    {
        m_CurrentObject->pVerts[i].x -= m_CurrentObject->vPosition[0].x + m_CurrentObject->vPivot.x;
        m_CurrentObject->pVerts[i].y -= m_CurrentObject->vPosition[0].y + m_CurrentObject->vPivot.y;
        m_CurrentObject->pVerts[i].z -= m_CurrentObject->vPosition[0].z + m_CurrentObject->vPivot.z;
    }
}

void Loaders::t3DSLoader::ReadKeyFrameRotations(t3DModel *pModel, tChunk *pPreviousChunk)
{
    short frameNumber = 0, flags = 0, rotkeys = 0, ignored = 0;
    long lunknown = 0;
    float rotationDegree = 0;
    vector<int> vFrameNumber;
    vector<float> vRotDegree;
    vector<CVector3> vRotation;
    int i;

    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);

    pPreviousChunk->bytesRead += fread(&(m_CurrentObject->rotationFrames), 1, sizeof(short), m_FilePointer);

    pPreviousChunk->bytesRead += fread(&ignored, 1,  sizeof(short), m_FilePointer);

    for(i = 0; i < m_CurrentObject->rotationFrames; i++)
    {
        vRotation.push_back(CVector3());

        pPreviousChunk->bytesRead += fread(&frameNumber, 1, sizeof(short), m_FilePointer);

        vFrameNumber.push_back(frameNumber);

        pPreviousChunk->bytesRead += fread(&lunknown, 1, sizeof(long), m_FilePointer);

        pPreviousChunk->bytesRead += fread(&rotationDegree, 1, sizeof(float), m_FilePointer);

        rotationDegree = rotationDegree * (180.0f / 3.14159f);

        vRotDegree.push_back(rotationDegree);

        pPreviousChunk->bytesRead += fread(&(vRotation[i]), 1, sizeof(CVector3), m_FilePointer);

        vRotation[i].x = RoundFloat(vRotation[i].x);
        vRotation[i].y = RoundFloat(vRotation[i].y);
        vRotation[i].z = RoundFloat(vRotation[i].z);

        float temp = vRotation[i].y;
        vRotation[i].y = -vRotation[i].z;
        vRotation[i].z = temp;

        vRotation[i].x *= -1;
    }

    m_CurrentObject->vRotation.push_back(vRotation[0]);

    m_CurrentObject->vRotDegree.push_back(vRotDegree[0]);

    int currentKey = 1;

    for(i = 1; i <= (pModel->numberOfFrames + 1); i++)
    {
        if(currentKey < m_CurrentObject->rotationFrames)
        {
            int currentFrame = vFrameNumber[currentKey];
            int previousFrame = vFrameNumber[currentKey - 1];
            float degree = vRotDegree[currentKey];

            float rotDegree = degree / (currentFrame - previousFrame);

            m_CurrentObject->vRotation.push_back(vRotation[currentKey]);
            m_CurrentObject->vRotDegree.push_back(rotDegree);

            if(vFrameNumber[currentKey] <= i)
                currentKey++;
        }
        else
        {
            m_CurrentObject->vRotation.push_back(vRotation[currentKey - 1]);
            m_CurrentObject->vRotDegree.push_back(0.0f);
        }
    }
}

void Loaders::t3DSLoader::ReadKeyFrameScales(t3DModel *pModel, tChunk *pPreviousChunk)
{
    short frameNumber = 0, ignore = 0, flags = 0;
    long lunknown = 0;
    int i = 0;

    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);
    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);

    pPreviousChunk->bytesRead += fread(&(m_CurrentObject->scaleFrames), 1, sizeof(short), m_FilePointer);

    pPreviousChunk->bytesRead += fread(&ignore, 1, sizeof(short), m_FilePointer);

    for(i = 0; i <= (pModel->numberOfFrames + 1); i++)
    {
        m_CurrentObject->vScale.push_back(CVector3());

        if(i < m_CurrentObject->scaleFrames)
        {
            pPreviousChunk->bytesRead += fread(&frameNumber, 1, sizeof(short), m_FilePointer);

            pPreviousChunk->bytesRead += fread(&lunknown, 1, sizeof(long), m_FilePointer);

            pPreviousChunk->bytesRead +=
            fread(&(m_CurrentObject->vScale[i]), 1, sizeof(CVector3), m_FilePointer);

            float temp = m_CurrentObject->vScale[i].y;
            m_CurrentObject->vScale[i].y = m_CurrentObject->vScale[i].z;
            m_CurrentObject->vScale[i].z = temp;
        }
        else
            m_CurrentObject->vScale[i] = m_CurrentObject->vScale[m_CurrentObject->scaleFrames - 1];
    }
}

void Loaders::t3DSLoader::SetCurrentObject(t3DModel *pModel, char *strObjectName)
{
    if(!strObjectName)
    {
        m_CurrentObject = NULL;
        return;
    }

    for(int i = 0; i < pModel->numOfObjects; i++)
    {
        if(strcmp(pModel->pObject[i].strName, strObjectName) == 0)
        {
            m_CurrentObject =&(pModel->pObject[i]);
            return;
        }
    }

    MessageBox(NULL, "ERROR: No object in model with given name! (SetCurrentObject)", "Error!", MB_OK);

    m_CurrentObject = NULL;
}

void Loaders::t3DSLoader::ComputeNormals(t3DModel *pModel)
{
    CVector3 vVector1, vVector2, vNormal, vPoly[3];

    if(pModel->numOfObjects <= 0)
        return;

    for(int index = 0; index < pModel->numOfObjects; index++)
    {
        t3DObject *pObject = &(pModel->pObject[index]);

        CVector3 *pNormals        = new CVector3 [pObject->numOfFaces];
        CVector3 *pTempNormals    = new CVector3 [pObject->numOfFaces];
        pObject->pNormals        = new CVector3 [pObject->numOfVerts];

        for(int i=0; i < pObject->numOfFaces; i++)
        {
            vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];
            vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];
            vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];

            vVector1 = Vector(vPoly[0], vPoly[2]);
            vVector2 = Vector(vPoly[2], vPoly[1]);

            vNormal  = Cross(vVector1, vVector2);
            pTempNormals[i] = vNormal;
            vNormal  = Normalize(vNormal);

            pNormals[i] = vNormal;
        }

        CVector3 vSum(0.0, 0.0, 0.0);
        CVector3 vZero = vSum;
        int shared = 0;

        for (int i = 0; i < pObject->numOfVerts; i++)
        {
            for (int j = 0; j < pObject->numOfFaces; j++)
            {
                if (pObject->pFaces[j].vertIndex[0] == i ||
                    pObject->pFaces[j].vertIndex[1] == i ||
                    pObject->pFaces[j].vertIndex[2] == i)
                {
                    vSum = AddVector(vSum, pTempNormals[j]);
                    shared++;
                }
            }

            pObject->pNormals[i] = DivideVectorByScaler(vSum, float(-shared));

            pObject->pNormals[i] = Normalize(pObject->pNormals[i]);

            vSum = vZero;
            shared = 0;
        }

        delete [] pTempNormals;
        delete [] pNormals;
    }
}

void Loaders::LoadModel(uint32 id)
{
    // Neexistuje
    if (sStorage->Models.find(id) == sStorage->Models.end())
        return;

    // Uz je nacten
    if (sStorage->Models[id] != NULL)
        return;

    t3DModel* dest = new t3DModel;
    char* path = new char[2048];
    sprintf(path, "%s/%s",DATA_PATH, sStorage->ModelFilename[id].c_str());

    fprintf(stdout, "Nacitani modelu %u: %s\n", id, path);

    ModelLoader.Import3DS(dest, path);

    for (uint32 i = 0; i < dest->pMaterials.size(); i++)
    {
        if (dest->pMaterials[i].strFile && strlen(dest->pMaterials[i].strFile) > 1)
        {
            memset(path, 0, 2048);
            sprintf(path, "%s/%s", DATA_PATH, dest->pMaterials[i].strFile);
            Loaders::LoadGLImage(&dest->pMaterials[i].texureId, path);
        }
    }

    if (dest)
    {
        GenDisplayLists(dest, id);
        sStorage->Models[id] = dest;
    }
}

void Loaders::GenDisplayLists(t3DModel* pModel, uint32 modelId)
{
    if (pModel->displayListSize == 0)
    {
        glLoadIdentity();

        uint32 maxFrame = 0;
        for (uint32 an = 0; an < MAX_ANIM; an++)
            if (maxFrame < sStorage->ModelAnimation[modelId].Anim[an].frameLast)
                maxFrame = sStorage->ModelAnimation[modelId].Anim[an].frameLast;

        pModel->displayList = glGenLists(maxFrame+1);
        pModel->displayListSize = maxFrame + 1;

        for (uint32 a = 0; a <= maxFrame; a++)
        {
            glNewList(pModel->displayList + a,GL_COMPILE);
            for(int i = 0; i < pModel->numOfObjects; i++)
            {
                if (pModel->pObject.size() <= 0) break;
                t3DObject *pObject = &pModel->pObject[i];

                glPushMatrix();
                sDisplay->AnimateModelObjectByFrame(pModel, pObject, modelId, a);

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

                /*
                glShadeModel(GL_SMOOTH);
                GLfloat mShininess[] = {2+4};
                GLfloat mSpecular[] = {0.4f, 0.4f, 0.4f, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mSpecular);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mShininess);
                */

                glBegin(GL_TRIANGLES);

                for(int j = 0; j < pObject->numOfFaces; j++)
                {
                    for(int whichVertex = 0; whichVertex < 3; whichVertex++)
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

                            // Pokud existuji modifiery pro opakovatelnost textur, nacteme je
                            float mod_x = 1.0f, mod_y = 1.0f;
                            if (pObject->strName)
                            {
                                if (ObjectModifierData* pMod = sStorage->GetObjectModifierData(modelId, &(pObject->strName[0])))
                                {
                                    mod_x = pMod->texture_repeat_x;
                                    mod_y = pMod->texture_repeat_y;
                                }
                            }

                            glTexCoord2f(pTexVert->x * mod_x, pTexVert->y * mod_y);
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
            glEndList();
        }
    }
}
