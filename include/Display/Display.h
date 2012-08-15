#ifndef BOMB_DISPLAY_H
#define BOMB_DISPLAY_H

#include <Global.h>
#include <Singleton.h>
#include <Storage.h>
#include <Config.h>

struct DisplayListRecord;
struct ModelDisplayListRecord;
struct BillboardDisplayListRecord;

#include <Effects/Animations.h>
#include <Effects/ParticleEmitter.h>

#define WIDTH sConfig->WindowWidth
#define HEIGHT sConfig->WindowHeight
#define WIDTHPCT WIDTH/100
#define HEIGHTPCT HEIGHT/100

#define IN_RANGE(a,b,c,d,e,f) (a > c && a < d && b > e && b < f)

enum Fonts
{
    MAIN_FONT  = 0,
    FONT_ONE   = 1,
    MAX_FONTS
};

static const uint32 fontTextures[MAX_FONTS] = {17, 44};

#define FONT_SIZE_H5 2.0f
#define FONT_SIZE_H4 1.8f
#define FONT_SIZE_H3 1.6f
#define FONT_SIZE_H2 1.4f
#define FONT_SIZE_H1 1.2f
#define FONT_SIZE_N 1.0f
#define FONT_SIZE_1 0.8f
#define FONT_SIZE_2 0.6f
#define FONT_SIZE_3 0.4f
#define FONT_SIZE_4 0.3f
#define FONT_SIZE_5 0.2f

//kolize s osami
enum AxisColission
{
    AXIS_X = 0x1,
    AXIS_Y = 0x2,
    AXIS_Z = 0x4
};

// Vzdalenost modelu pri kolizi
#define COLLISION_RANGE 0.2f

// Display List zaznamy

enum DisplayListType
{
    DL_TYPE_NONE      = 0,
    DL_TYPE_MODEL     = 1,
    DL_TYPE_BILLBOARD = 2,
    DL_TYPE_MAX
};

// typy doplnku modelu
enum ModelFeatureType
{
    MF_TYPE_MODEL     = 0,
    MF_TYPE_BILLBOARD = 1,
    MF_TYPE_EMITTER   = 2,
    MF_TYPE_MAX
};

/** \struct ModelFeature
 *  \brief Struktura zaznamu featury modelu
 */
struct ModelFeature
{
    ModelFeature()
    {
        type = MF_TYPE_MAX;
        feature = NULL;

        offset_x = 0.0f;
        offset_y = 0.0f;
        offset_z = 0.0f;
    }

    ModelFeatureType type;
    void* feature;
    float offset_x, offset_y, offset_z;

    ModelDisplayListRecord* ToModel()
    {
        if (type == MF_TYPE_MODEL)
            return (ModelDisplayListRecord*)feature;
        else
            return NULL;
    }
    BillboardDisplayListRecord* ToBillboard()
    {
        if (type == MF_TYPE_BILLBOARD)
            return (BillboardDisplayListRecord*)feature;
        else
            return NULL;
    }
    Emitter* ToEmitter()
    {
        if (type == MF_TYPE_EMITTER)
            return (Emitter*)feature;
        else
            return NULL;
    }
};

/** \var ModelFeature::type
 *  \brief Urcuje typ featury modelu, viz enumerator ModelFeatureType
 */

/** \var ModelFeature::feature
 *  \brief Ukazatel do pameti na zaznam featury. Cil ukazatele se lisi podle typu
 */

/** \var ModelFeature::offset_x
 *  \brief Posunuti featury po ose X relativne od pozice modelu, ke kteremu patri
 */

/** \var ModelFeature::offset_y
 *  \brief Posunuti featury po ose Y relativne od pozice modelu, ke kteremu patri
 */

/** \var ModelFeature::offset_z
 *  \brief Posunuti featury po ose Z relativne od pozice modelu, ke kteremu patri
 */

/** \fn ModelFeature::ToModel
 *  \brief Stara se o konverzi ukazatele *feature na displaylist record modelu
 *
 *  Vraci ukazatel na displaylist record modelu, pokud typ neodpovida, vraci NULL
 */

/** \fn ModelFeature::ToBillboard
 *  \brief Stara se o konverzi ukazatele *feature na displaylist record billboardu
 *
 *  Vraci ukazatel na displaylist record billboardu, pokud typ neodpovida, vraci NULL
 */

/** \fn ModelFeature::ToEmitter
 *  \brief Stara se o konverzi ukazatele *feature na ukazatel na emitter
 *
 *  Vraci ukazatel na emitter, pokud typ neodpovida, vraci NULL
 */

typedef std::list<ModelFeature*> FeatureList;

// animacni restrikce (napr. pri pauznuti hry, ..)
enum AnimRestriction
{
    ANIM_RESTRICTION_NONE = 0,
    ANIM_RESTRICTION_NOT_PAUSED = 1,
    ANIM_RESTRICTION_MAX = 2
};

// flagy pro text
enum TextFlags
{
    TEXT_FLAG_CENTERED_X    = 0x01,
    TEXT_FLAG_CENTERED_Y    = 0x02,
};

/** \struct DisplayListRecord
 *  \brief Rodicovska trida vsech displaylist zaznamu
 */
struct DisplayListRecord
{
    DisplayListRecord(uint32 type)
    {
        m_type = type;
        displayList = 0;
        displayListSize = 0;
        insertionTime = 0;
    }

    float  x, y, z;
    uint32 m_type;
    bool   remove;
    GLDisplayList displayList;
    uint32 displayListSize;
    uint32 AnimTicket;
    AnimRestriction animRestriction;
    clock_t insertionTime;
};

/** \var DisplayListRecord::x
 *  \brief Pozice zaznamu na ose X
 */

/** \var DisplayListRecord::y
 *  \brief Pozice zaznamu na ose Y
 */

/** \var DisplayListRecord::z
 *  \brief Pozice zaznamu na ose Z
 */

/** \var DisplayListRecord::m_type
 *  \brief Typ zaznamu (kvuli dedicnosti)
 */

/** \var DisplayListRecord::remove
 *  \brief Priznak, zdali je zaznam urcen k vymazani.
 *
 * Nemuzeme mazat primo pri pozadavku, musime nechat update funkci (hlavni vlakno) vymazat zaznam, jinak muze dojit k nekonzistenci
 */

/** \var DisplayListRecord::displayList
 *  \brief Generovany OpenGL display list (pokud je nutne generovat zvlast)
 */

/** \var DisplayListRecord::displayListSize
 *  \brief Velikost OpenGL display listu, ktery mohl byt generovan zvlast. Zaroven slouzi jako priznak, jestli byl vubec vygenerovan (velikost 0 = nebyl)
 */

/** \var DisplayListRecord::AnimTicket
 *  \brief Ulozeny ticket pro animace (podle toho se pote zvoli snimek animace a potazmo OpenGL display list, ktery se ma vykreslit)
 */

/** \var DisplayListRecord::animRestriction
 *  \brief Restrikce pro animaci, diky ktere muzeme zastavit napriklad vsechny animace z dane skupiny najednou (napr. pauza hry = zastaveni animace vsech hernich objektu)
 */

/** \var DisplayListRecord::insertionTime
 *  \brief Cas vlozeni. Slouzi jako orientacni hodnota a take k nekterym custom animacim
 */


/** \struct ModelDisplayListRecord
 *  \brief Struktura zaznamu displaylistu modelu
 */
struct ModelDisplayListRecord: public DisplayListRecord
{
    ModelDisplayListRecord(): DisplayListRecord(DL_TYPE_MODEL)
    {
        remove = false;
        AnimTicket = 0;
        artkit = 0;
    }

    uint32 modelId;
    uint32 CustomFrame;
    float  scale, rotate;
    FeatureList features;
    uint32 artkit;
};

/** \var ModelDisplayListRecord::modelId
 *  \brief ID zobrazovaneho modelu
 */

/** \var ModelDisplayListRecord::CustomFrame
 *  \brief Slouzi k predavani udaju mezi jednotlivymi nezavislymi procedurami
 */

/** \var ModelDisplayListRecord::scale
 *  \brief Vlastni zvetseni daneho modelu
 */

/** \var ModelDisplayListRecord::rotate
 *  \brief Otoceni po vektoru osy Y (v OpenGL souradnem systemu, v matematice a deskriptivni geometrii by to byla osa Z)
 */

/** \var ModelDisplayListRecord::features
 *  \brief List veskerych featur daneho zaznamu modelu
 */

/** \var ModelDisplayListRecord::artkit
 *  \brief ID barevne varianty modelu
 */


/** \struct BillboardDisplayListRecord
 *  \brief Struktura zaznamu displaylistu billboardu
 */
struct BillboardDisplayListRecord: public DisplayListRecord
{
    BillboardDisplayListRecord(): DisplayListRecord(DL_TYPE_BILLBOARD)
    {
        remove = false;
        AnimTicket = 0;
        rotate_x = rotate_y = rotate_z = 0.0f;
    }

    static BillboardDisplayListRecord* Create(uint32 textureId, float x, float y, float z,
                                              float scale_x = 1.0f, float scale_y = 1.0f,
                                              bool billboard_x = true, bool billboard_y = true);

    uint32 textureId;
    float  scale_x, scale_y;
    float  rotate_x, rotate_y, rotate_z;
    bool   billboard_x, billboard_y;
};

/** \fn BillboardDisplayListRecord::Create
 *  \brief Staticka metoda pro vytvoreni zaznamu, ale nevlozeni do samotneho listu k vykresleni
 *
 * Slouzi napriklad pri poskytovani predlohy pro castice pro emitter
 */

/** \var BillboardDisplayListRecord::textureId
 *  \brief ID textury k vykresleni
 */

/** \var BillboardDisplayListRecord::scale_x
 *  \brief Sirka vykresleneho zaznamu
 */

/** \var BillboardDisplayListRecord::scale_y
 *  \brief Vyska vykresleneho zaznamu
 */

/** \var BillboardDisplayListRecord::billboard_x
 *  \brief Priznak pro nataceni kolmo ke kamere po ose X
 */

/** \var BillboardDisplayListRecord::billboard_y
 *  \brief Priznak pro nataceni kolmo ke kamere po ose Y
 */

#define COLOR(r,g,b) uint32(uint32(uint8(r) << 24)|uint32(uint8(g) << 16)|uint32(uint8(b) << 8)|uint32(uint8(0)))
#define COLORA(r,g,b,a) uint32(uint32(uint8(r) << 24)|uint32(uint8(g) << 16)|uint32(uint8(b) << 8)|uint32(uint8(a)))

#define NOCOLOR COLOR(255,255,255)

#define CRED(x)   uint8(x >> 24)
#define CGREEN(x) uint8(x >> 16)
#define CBLUE(x)  uint8(x >> 8)
#define CALPHA(x) uint8(x)

/** \class DisplayMgr
 *  \brief Trida starajici se o zobrazeni, vykreslovani a spravu zobrazovaneho obsahu
 */
class DisplayMgr
{
    public:
        DisplayMgr();
        ~DisplayMgr();

        // Inicializace vseho mozneho
        void StartUp();
        void Initialize();
        void InitFont(uint8 font);

        // Update funkce co se vola cely loop
        void Update();

        // Vse co se tyce vykreslovani modelu
        ModelDisplayListRecord* DrawModel(uint32 modelId, float x, float y, float z,
                                          ModelAnimType Animation = ANIM_IDLE,
                                          float scale = 1.0f, float rotate = 0.0f,
                                          bool genGLDisplayList = false, bool animReverse = false,
                                          uint32 startFrame = 0, uint32 frameSkipSpeed = 0, AnimRestriction animRest = ANIM_RESTRICTION_NONE,
                                          bool GLDisplayListOnly = false, uint32 artkit = 0);
        bool RemoveRecordFromDisplayList(ModelDisplayListRecord* target);
        void DrawModels();
        void AddModelFeature(ModelDisplayListRecord* record, ModelFeatureType type, float offset_x, float offset_y, float offset_z, void* feature);
        void ClearModelFeatures(ModelDisplayListRecord* record);
        void ClearModelFeaturesByType(ModelDisplayListRecord* record, ModelFeatureType type, bool hard = false);
        void ClearAllModelFeaturesByType(ModelFeatureType type, bool hard = false);
        void AnimateModelObject(t3DObject *object, ModelDisplayListRecord* pData);
        void AnimateModelObjectByFrame(t3DObject *object, uint32 modelId, uint32 frame);
        void AnimateModelObjectByFrame(t3DModel* model, t3DObject *object, uint32 modelId, uint32 frame);
        void FlushModelDisplayList();
        bool ModelIntersection(ModelDisplayListRecord* first, ModelDisplayListRecord* second);

        // Vse co se tyce vykreslovani billboardu
        BillboardDisplayListRecord* DrawBillboard(uint32 textureId, float x, float y, float z,
                                                  uint32 Animation = ANIM_NONE, uint32 animFrameSpeed = 1,
                                                  float scale_x = 1.0f, float scale_y = 1.0f,
                                                  bool billboard_x = true, bool billboard_y = true,
                                                  bool genGLDisplayList = false, AnimRestriction animRest = ANIM_RESTRICTION_NONE,
                                                  uint8 animFlags = 0);
        bool RemoveRecordFromDisplayList(BillboardDisplayListRecord* target);
        void DrawBillboards();
        void FlushBillboardDisplayList();

        void DisableRestrictedAnimations(AnimRestriction animRes);
        void EnableRestrictedAnimations(AnimRestriction animRes);

        // Prechod mezi 2D a 3D mody
        void Setup2DMode();
        void Setup3DMode();
        bool IsIn2DMode() { return m_is2D; };

        // Text
        void PrintText(uint8 font, uint32 left, uint32 top, float scale, uint8 flags, uint32 color, const char* fmt, ...);
        void PrintParagraphText(uint8 font, uint32 left, uint32 top, uint32 width, float scale, uint8 flags, uint32 color, const char* fmt, ...);

        // 2D funkce
        void Draw2D(uint32 textureId, float left, float top, float width, float height);

        // Prace s mapou
        void DrawMap();

        // Prace s objektem pozorovani
        void AdjustViewToTarget();
        uint16 CheckCollision(float newx, float newy, float newz);

        // Interfacing s privatnimi promennymi
        void SetViewX(float val, bool relative = false) { m_viewX = (relative ? (m_viewX + val) : (val)); };
        void SetViewY(float val, bool relative = false) { m_viewY = (relative ? (m_viewY + val) : (val)); };
        void SetViewZ(float val, bool relative = false) { m_viewZ = (relative ? (m_viewZ + val) : (val)); };
        void SetAngleX(float val, bool relative = false) { m_angleX = (relative ? (m_angleX + val) : (val)); };
        void SetAngleY(float val, bool relative = false) { m_angleY = (relative ? (m_angleY + val) : (val)); };
        void SetAngleZ(float val, bool relative = false) { m_angleZ = (relative ? (m_angleZ + val) : (val)); };
        void SetTargetX(float val, bool relative = false) { m_targetX = (relative ? (m_targetX + val) : (val)); };
        void SetTargetY(float val, bool relative = false) { m_targetY = (relative ? (m_targetY + val) : (val)); };
        void SetTargetZ(float val, bool relative = false) { m_targetZ = (relative ? (m_targetZ + val) : (val)); };
        void SetTargetAngleX(float val, bool relative = false) { m_tarangleX = (relative ? (m_tarangleX + val) : (val)); };
        void SetTargetAngleY(float val, bool relative = false) { m_tarangleY = (relative ? (m_tarangleY + val) : (val)); };
        void SetTargetAngleZ(float val, bool relative = false) { m_tarangleZ = (relative ? (m_tarangleZ + val) : (val)); };
        void SetTargetModel(ModelDisplayListRecord* pTarget);

        float GetViewX() { return m_viewX; };
        float GetViewY() { return m_viewY; };
        float GetViewZ() { return m_viewZ; };
        float GetAngleX() { return m_angleX; };
        float GetAngleY() { return m_angleY; };
        float GetAngleZ() { return m_angleZ; };
        float GetTargetX() { return m_targetX; };
        float GetTargetY() { return m_targetY; };
        float GetTargetZ() { return m_targetZ; };
        float GetTargetAngleX() { return m_tarangleX; };
        float GetTargetAngleY() { return m_tarangleY; };
        float GetTargetAngleZ() { return m_tarangleZ; };
        ModelDisplayListRecord* GetTargetModel() { return m_targetmodel; };

        void DeviateHorizontalAngle(float angle) { m_deviateAngleY = angle; };
        float GetHorizontalAngleDeviation() { return m_deviateAngleY; };

        // Vyjimky pro sledovany model - z vnejsku pristupne primo
        uint32 m_ignoreTargetCollision;

        bool BindTexture(uint32 textureId);
    private:
        // Display list modelu
        list<ModelDisplayListRecord*> ModelDisplayList;
        // Display list billboardu
        list<BillboardDisplayListRecord*> BillboardDisplayList;

        // Jsme ve 2D rezimu? Defaultne false
        bool m_is2D;
        // Promenna ktera uchovava stav zapnuti svetel pro pozdejsi obnoveni
        bool m_cachedLighting;

        // Promenne specifikujici pozici a natoceni kamery
        float m_viewX;
        float m_viewY;
        float m_viewZ;
        float m_angleX;
        float m_angleY;
        float m_angleZ;

        // Promenne specifikujici pozici a uhly sledovaneho objektu
        float m_targetX;
        float m_targetY;
        float m_targetZ;
        float m_tarangleX;
        float m_tarangleY;
        float m_tarangleZ;
        ModelDisplayListRecord* m_targetmodel;

        float m_deviateAngleY;

        // Font
        GLuint m_fontBase[MAX_FONTS];
        bool m_fontLoaded[MAX_FONTS];

        // Ruzne pracovni promenne
        uint32 m_boundTexture;
};

#define sDisplay Singleton<DisplayMgr>::instance()

/** \fn DisplayMgr::IsIn2DMode()
 *  \brief Vraci true, pokud jsme ve 2D rezimu
 */

/** \fn DisplayMgr::DeviateHorizontalAngle(float angle)
 *  \brief Postara se o nastaveni horizontalni odchylky uhlu (ve stupnich)
 */

/** \fn DisplayMgr::GetHorizontalAngleDeviation()
 *  \brief Vraci hodnotu horizontalni odchylky uhlu
 */

/** \fn DisplayMgr::GetTargetModel()
 *  \brief Vraci ukazatel na zaznam v display listu, ke kteremu je prizpusobovana kamera
 */

#endif
