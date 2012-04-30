#ifndef BOMB_3DS_H
#define BOMB_3DS_H

#include <Global.h>

#define MODEL_SCALE 0.1f

#define PERCENT_CHUNK_INT   0x0030
#define PERCENT_CHUNK_FLOAT 0x0031

#define PRIMARY       0x4D4D
#define OBJECTINFO    0x3D3D
#define VERSION       0x0002
#define MATERIAL      0xAFFF
#define OBJECT        0x4000
#define MATNAME       0xA000
#define MATDIFFUSE    0xA020
#define MATMAP        0xA200
#define MATMAPFILE    0xA300

#define MATSPECULAR      0xA030
#define MATSHININESS     0xA040
#define MATSHININESSSTR  0xA041
#define MATSHININESSSTR2 0xA042

#define OBJECT_MESH       0x4100
#define OBJECT_VERTICES   0x4110
#define OBJECT_FACES      0x4120
#define OBJECT_MATERIAL   0x4130
#define OBJECT_UV         0x4140

#define KEYFRAME                0xB000
#define KEYFRAME_MESH_INFO      0xB002
#define KEYFRAME_OBJECT_NAME    0xB010
#define KEYFRAME_START_AND_END  0xB008
#define PIVOT                   0xB013
#define POSITION_TRACK_TAG      0xB020
#define ROTATION_TRACK_TAG      0xB021
#define SCALE_TRACK_TAG         0xB022

/** \struct tChunk
 *  \brief Hlavicka datove "bunky" souboru modelu
 */
struct tChunk
{
    uint16 ID;
    uint32 length;
    uint32 bytesRead;
};

/** \var tChunk::ID
 *  \brief ID daneho nacteneho kusu modelu
 */

/** \var tChunk::length
 *  \brief Delka dat, ktere nasleduji (v bajtech)
 */

/** \var tChunk::bytesRead
 *  \brief Pocet bajtu, ktere jiz byly precteny (jakmile bytesRead == length, cte se dalsi chunk)
 */


/** \struct tFace
 *  \brief Struktura jedne steny objektu
 */
struct tFace
{
    int32 vertIndex[3];
    int32 coordIndex[3];
};

/** \var tFace::vertIndex
 *  \brief Index vertexu dane face
 */

/** \var tFace::coordIndex
 *  \brief Index souradnic dane face
 */


/** \struct tMaterialInfo
 *  \brief Struktura s informacemi o materialu objektu
 */
struct tMaterialInfo
{
    char   strName[255];
    char   strFile[255];
    uint8  color[3];
    uint8  specularcolor[3];
    uint32 textureId;
    float  uTile;
    float  vTile;
    float  uOffset;
    float  vOffset;

    /** \union tMaterialInfo::shininess
     *  \brief Lesklost materialu
     */
    union shininess
    {
        float fShininess[3];
        uint8 uShininess[3];
    } shininess;
};

/** \var tMaterialInfo::strName
 *  \brief Jmeno materialu
 */

/** \var tMaterialInfo::strFile
 *  \brief Jmeno souboru s texturou
 */

/** \var tMaterialInfo::color
 *  \brief Barva materialu (R, G, B)
 */

/** \var tMaterialInfo::specularcolor
 *  \brief Specularni (odleskova) barva daneho materialu (R, G, B)
 */

/** \var tMaterialInfo::textureId
 *  \brief ID nactene OpenGL textury
 */

/** \var tMaterialInfo::uTile
 *  \brief Nepouzivane
 */

/** \var tMaterialInfo::vTile
 *  \brief Nepouzivane
 */

/** \var tMaterialInfo::uOffset
 *  \brief Nepouzivane
 */

/** \var tMaterialInfo::vOffset
 *  \brief Nepouzivane
 */

/** \var tMaterialInfo::shininess
 *  \brief Lesklost materialu
 */

/** \var tMaterialInfo::shininess::uShininess
 *  \brief Lesklost materialu udavana v 8bitovych bezznaminkovych cislech
 */

/** \var tMaterialInfo::shininess::fShininess
 *  \brief Lesklost materialu udavana v procentech
 */


/** \struct t3DObject
 *  \brief Struktura objektu modelu
 */
struct t3DObject
{
    int32 numOfVerts;
    int32 numOfFaces;
    int32 numTexVertex;
    int32 materialID;
    bool  bHasTexture;
    char  strName[255];

    int32 positionFrames;
    int32 rotationFrames;
    int32 scaleFrames;

    CVector3 vPivot;

    vector<CVector3> vPosition;
    vector<CVector3> vRotation;
    vector<CVector3> vScale;

    vector<float> vRotDegree;

    CVector3 *pVerts;
    CVector3 *pNormals;
    CVector2 *pTexVerts;
    tFace *pFaces;
};

/** \var t3DObject::numOfVerts
 *  \brief Pocet vertexu v danem objektu
 */

/** \var t3DObject::numOfFaces
 *  \brief Pocet sten v danem objektu
 */

/** \var t3DObject::numTexVertex
 *  \brief Pocet texturovych vertexu v danem objektu
 */

/** \var t3DObject::materialID
 *  \brief ID materialu pouzivaneho na danem objektu
 */

/** \var t3DObject::bHasTexture
 *  \brief Priznak, zdali je objekt texturovany
 */

/** \var t3DObject::strName
 *  \brief Jmeno objektu
 */

/** \var t3DObject::positionFrames
 *  \brief Pocet snimku animace, starajici se o zmenu pozice objektu
 */

/** \var t3DObject::rotationFrames
 *  \brief Pocet snimku animace, starajici se o natoceni objektu
 */

/** \var t3DObject::scaleFrames
 *  \brief Pocet snimku animace, starajici se o velikost objektu
 */

/** \var t3DObject::vPivot
 *  \brief Pivot ("relativni stred otaceni") objektu
 */

/** \var t3DObject::vPosition
 *  \brief Animacni data snimku menici pozici objektu
 */

/** \var t3DObject::vRotation
 *  \brief Animacni data snimku menici natoceni objektu
 */

/** \var t3DObject::vScale
 *  \brief Animacni data snimku menici velikost objektu
 */

/** \var t3DObject::vRotDegree
 *  \brief Prepocitane hodnoty z vRotation z radianu na stupne
 */

/** \var t3DObject::pVerts
 *  \brief Vsechny vertexy objektu
 */

/** \var t3DObject::pNormals
 *  \brief Vsechny vertexove normaly objektu
 */

/** \var t3DObject::pTexVerts
 *  \brief Vsechny texturove vertexy objektu
 */

/** \var t3DObject::pFaces
 *  \brief Vsechny steny objektu
 */


/** \struct t3DModel
 *  \brief Struktura modelu
 */
struct t3DModel
{
    t3DModel()
    {
        displayList = 0;
        displayListSize = 0;
        displayListArtkit.clear();
        numOfObjects = 0;
        numOfMaterials = 0;
        numberOfFrames = 1;
        pMaterials.clear();
        pObject.clear();
        Maximum.x = 0; Minimum.x = 0;
        Maximum.y = 0; Minimum.y = 0;
        Maximum.z = 0; Minimum.z = 0;
    }
    int32 numOfObjects;
    int32 numOfMaterials;

    int32 numberOfFrames;

    vector<tMaterialInfo> pMaterials;
    vector<t3DObject> pObject;
    vector<float> customScale;

    CVector3 Maximum;
    CVector3 Minimum;

    GLuint displayList;
    uint32 displayListSize;
    std::vector<GLuint> displayListArtkit;
};

/** \fn t3DModel::t3DModel
 *  \brief Konstruktor
 *
 * Nulovani vsech promennych a ukazatelu
 */

/** \var t3DModel::numOfObjects
 *  \brief Pocet objektu daneho modelu
 */

/** \var t3DModel::numOfMaterials
 *  \brief Pocet materialu spojenych s danym modelem
 */

/** \var t3DModel::numberOfFrames
 *  \brief Celkovy pocet snimku animace
 */

/** \var t3DModel::pMaterials
 *  \brief Pole vsech materialu objektu
 */

/** \var t3DModel::pObject
 *  \brief Pole vsech objektu
 */

/** \var t3DModel::customScale
 *  \brief Vlastni velikost modelu generovana napriklad ve tride CustomAnimator
 */

/** \var t3DModel::Maximum
 *  \brief Maximalni bod modelu (nemodifikovany globalnim a vlastnim modifikatorem velikosti)
 */

/** \var t3DModel::Minimum
 *  \brief Minimalni bod modelu (nemodifikovany globalnim a vlastnim modifikatorem velikosti)
 */

/** \var t3DModel::displayList
 *  \brief ID vygenerovaneho OpenGL display listu
 */

/** \var t3DModel::displayListSize
 *  \brief Velikost vygenerovaneho OpenGL display listu, nebo 0 pokud neni vygenerovany
 */

/** \var t3DModel::displayListArtkit
 *  \brief Pole vsech ID OpenGL displaylistu generovanych pro artkity
 */

namespace Loaders
{
    static class t3DSLoader
    {
    public:
        t3DSLoader();
        ~t3DSLoader();

        bool Import3DS(t3DModel *pModel, char *strFileName);
    private:
        int GetString(char *);
        void ReadChunk(tChunk *);
        void ProcessNextChunk(t3DModel *pModel, tChunk *);
        void ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *);
        void ProcessNextMaterialChunk(t3DModel *pModel, tChunk *);
        void ProcessNextKeyFrameChunk(t3DModel *pModel, tChunk *);
        void ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk);
        void ReadColorChunkDest(uint8 *dest, tChunk *pChunk);
        void ReadPercentageChunkDest(uint8 *dest, tChunk *pChunk);
        void ReadVertices(t3DObject *pObject, tChunk *);
        void ReadVertexIndices(t3DObject *pObject, tChunk *);
        void ReadUVCoordinates(t3DObject *pObject, tChunk *);
        void ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk);
        void ReadKeyFramePositions(t3DModel *pModel, tChunk *pPreviousChunk);
        void ReadKeyFrameRotations(t3DModel *pModel, tChunk *pPreviousChunk);
        void ReadKeyFrameScales(t3DModel *pModel, tChunk *pPreviousChunk);
        void SetCurrentObject(t3DModel *pModel, char *strObjectName);
        void ResizeObjects(t3DModel *pModel);
        void ComputeNormals(t3DModel *pModel);
        void StoreMaximumMinimum(t3DModel* pModel);
        void CleanUp();

        FILE *m_FilePointer;

        tChunk *m_CurrentChunk;
        tChunk *m_TempChunk;

        t3DObject *m_CurrentObject;
    } ModelLoader;

    void LoadModel(uint32 id);
    void GenDisplayLists(t3DModel* pModel, uint32 modelId);
};

#endif
