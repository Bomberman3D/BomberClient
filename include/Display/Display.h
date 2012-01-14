#ifndef BOMB_DISPLAY_H
#define BOMB_DISPLAY_H

#include <Global.h>
#include <Singleton.h>
#include <Storage.h>
#include <Config.h>

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

// Zakladni struktura pro vsechny
// vyuzitelne napriklad v particle emitter kodu - hybat muze jak modelem tak billboardem
struct DisplayListRecord
{
    DisplayListRecord(uint32 type)
    {
        m_type = type;
        displayList = 0;
        displayListSize = 0;
    }

    float  x, y, z;
    uint32 m_type;
    bool   remove;
    GLDisplayList displayList;
    uint32 displayListSize;
};

struct ModelDisplayListRecord: public DisplayListRecord
{
    ModelDisplayListRecord(): DisplayListRecord(DL_TYPE_MODEL)
    {
        remove = false;
        AnimTicket = 0;
    }

    uint32 modelId;
    uint32 AnimTicket;
    //bool  collision; // NYI
    float  scale, rotate;
};

struct BillboardDisplayListRecord: public DisplayListRecord
{
    BillboardDisplayListRecord(): DisplayListRecord(DL_TYPE_BILLBOARD)
    {
        remove = false;
        AnimTicket = 0;
    }

    static BillboardDisplayListRecord* Create(uint32 textureId, float x, float y, float z,
                                              float scale_x = 1.0f, float scale_y = 1.0f,
                                              bool billboard_x = true, bool billboard_y = true);

    uint32 textureId;
    uint32 AnimTicket;
    float  scale_x, scale_y;
    bool   billboard_x, billboard_y;
};

#define COLOR(r,g,b) uint32(uint32(uint8(r) << 24)|uint32(uint8(g) << 16)|uint32(uint8(b) << 8)|uint32(uint8(0)))
#define COLORA(r,g,b,a) uint32(uint32(uint8(r) << 24)|uint32(uint8(g) << 16)|uint32(uint8(b) << 8)|uint32(uint8(a)))

#define NOCOLOR COLOR(255,255,255)

#define CRED(x)   uint8(x >> 24)
#define CGREEN(x) uint8(x >> 16)
#define CBLUE(x)  uint8(x >> 8)
#define CALPHA(x) uint8(x)

class Display
{
    public:
        Display();
        ~Display();

        // Inicializace vseho mozneho
        void Initialize();
        void InitFont(uint8 font);

        // Update funkce co se vola cely loop
        void Update(const uint32 diff);

        // Vse co se tyce vykreslovani modelu
        ModelDisplayListRecord* DrawModel(uint32 modelId, float x, float y, float z,
                                          ModelAnimType Animation = ANIM_IDLE,
                                          float scale = 1.0f, float rotate = 0.0f,
                                          bool genGLDisplayList = false, bool animReverse = false,
                                          uint32 startFrame = 0, uint32 frameSkipSpeed = 0);
        bool RemoveRecordFromDisplayList(ModelDisplayListRecord* target);
        void DrawModels();
        void AnimateModelObject(t3DObject *pObject, ModelDisplayListRecord* pData);
        void AnimateModelObjectByFrame(t3DObject *pObject, ModelDisplayListRecord* pData, uint32 frame);
        void FlushModelDisplayList();

        // Vse co se tyce vykreslovani billboardu
        BillboardDisplayListRecord* DrawBillboard(uint32 textureId, float x, float y, float z,
                                                  uint32 Animation = ANIM_NONE, uint32 animFrameSpeed = 1,
                                                  float scale_x = 1.0f, float scale_y = 1.0f,
                                                  bool billboard_x = true, bool billboard_y = true,
                                                  bool genGLDisplayList = false);
        bool RemoveRecordFromDisplayList(BillboardDisplayListRecord* target);
        void DrawBillboards();
        void FlushBillboardDisplayList();

        // Prechod mezi 2D a 3D mody
        void Setup2DMode();
        void Setup3DMode();
        bool IsIn2DMode() { return m_is2D; };

        // Text
        void PrintText(uint8 font, uint32 left, uint32 top, float scale, uint32 color, const char* fmt, ...);

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

        void BindTexture(uint32 textureId);
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

#define sDisplay Singleton<Display>::instance()

#endif
