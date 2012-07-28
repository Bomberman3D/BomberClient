#ifndef BOMB_GAMEPLAY_H
#define BOMB_GAMEPLAY_H

#include <Global.h>
#include <Singleton.h>
#include <GameTypes.h>
#include <AI.h>
#include <SoundMgr.h>

enum SettingsEnum
{
    SETTING_ENEMY_COUNT = 0,
    SETTING_ENEMY_AI_LEVEL = 1,
    SETTING_ENEMY_SPEED = 2,
    SETTING_BOX_DENSITY = 3,
    SETTING_MAP_ID = 4,
    SETTING_MAX
};

enum PlayerMoveElements
{
    MOVE_FORWARD,
    MOVE_RIGHT,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_MAX
};

/** \struct BombRecord
 *  \brief Strukura zaznamu bomby
 */
struct BombRecord
{
    uint32 x;
    uint32 y;
    uint32 reach;
    clock_t boomTime;
    //pozdeji i data o vlastnikovi?

    SoundEffectRecord* sizzleSound;
};

/** \var BombRecord::x
 *  \brief Xova pozice bomby v 2D mape
 */

/** \var BombRecord::y
 *  \brief Yova pozice bomby v 2D mape
 */

/** \var BombRecord::reach
 *  \brief Dosah bomby po vsech smerech
 */

/** \var BombRecord::boomTime
 *  \brief Cas, kdy ma bomba vybouchnout [ms]
 */

/** \var BombRecord::sizzleSound
 *  \brief Odkaz na zvukovy efekt syceni bomby, pro pripadne preruseni
 */


/** \struct DangerousField
 *  \brief Struktura pro "nebezpecne pole"
 */
struct DangerousField
{
    clock_t activeSince;
    uint32  activeTime;
    bool    registered;
    BombRecord* origin;
};

/** \var DangerousField::activeSince
 *  \brief Cas, odkdy je bomba aktivni (napr. kvuli posupnemu dosahu plamene) [ms]
 */

/** \var DangerousField::activeTime
 *  \brief Cas, na jak dlouho je bomba aktivni [ms]
 */

/** \var DangerousField::registered
 *  \brief Priznak provedeni veci "po aktivaci", napr. zniceni beden apod.
 */

/** \var DangerousField::origin
 *  \brief Zdrojovy zaznam bomby, dulezite napr. kvuli retezovym vybuchum
 */

/** \union PlayerStats
 *  \brief Union struktur statistik hrace v dane hre
 */
typedef union
{
    struct UniversalStatTemplate
    {
        uint32 field1;
        uint32 field2;
        uint32 field3;
    } UniversalStats;

    struct
    {
        uint32 bombsPlanted;
        uint32 bonusesEarned;
        uint32 enemiesKilled;
    } ClassicSingleStats;

    struct
    {
        uint32 bombsPlanted;
        uint32 boxesDestroyed;
        uint32 enemiesTrolled;
    } MemeSingleStats;

    // TODO: dalsi typy her

    // NOTE: pri zmene poctu sloupcu nezapomenout zmenit konstantu o kousek niz
} PlayerStats;

#define MAX_PLAYER_STATS 3

// Nazvy sloupcu
// pouzit nulovy znak pro "neexistujici" pole
static const char* PlayerStatsNames[GAME_TYPE_MAX][MAX_PLAYER_STATS] = {
    {'\0','\0','\0'},
    // SP
    {"Položeno bomb","Sebráno bonusù","Zabito nepøátel"},
    {"Položeno bomb","Znièeno beden","Nepøátel trollnuto"},
    // MP
    {"Položeno bomb","Sebráno bonusù","Zabito nepøátel"},
    {"Položeno bomb","Znièeno beden","Nepøátel trollnuto"}
    //{'\0','\0','\0'}
};

// Vycet existujicich "cheatu"
enum CheatTypes
{
    CHEAT_IMMORTAL                 = 0,
    CHEAT_UNLIMITED_BOMBS          = 1,
    CHEAT_MAX_FLAME                = 2,
    CHEAT_MAX_SPEED                = 3,
    CHEAT_NEXT_LEVEL               = 4,
    CHEAT_KILL_ALL                 = 5,
    CHEAT_WALLHACK                 = 6,
    MAX_CHEAT,
};

#define CONSOLE_OUTPUT_LINES 5
#define CONSOLE_INPUT_HISTORY 10

// Struktura popisniku cheatu
struct ToggleCheatDesc
{
    uint32 id;        // id, viz CheatTypes
    std::string name; // jmeno, zobrazi se jako vystup v konzoli
    std::string str;  // co se musi zadat do konzole
};

static ToggleCheatDesc toggleCheatMap[] = {
    {CHEAT_IMMORTAL,        "Immortality",     "REDHEAD"},
    {CHEAT_UNLIMITED_BOMBS, "Unlimited bombs", "BIGPOCKET"},
    {CHEAT_MAX_FLAME,       "Max flame",       "FIRESTARTER"},
    {CHEAT_MAX_SPEED,       "Max speed",       "GONZALES"},
    {CHEAT_WALLHACK,        "Wallhack",        "HAXORZ"},
};


#define DEFAULT_BOMB_FLAME_DURATION 800

/** \class GameplayMgr
 *  \brief Trida starajici se o herni principy a specificke mechanizmy
 */
class GameplayMgr
{
    public:
        GameplayMgr();
        ~GameplayMgr();

        void Update();
        void SetGameType(GameType type);
        GameType GetGameType();

        bool IsSingleGameType()
        {
            return (GetGameType() >= GAME_TYPE_SP_MIN && GetGameType() <= GAME_TYPE_SP_MAX);
        }

        void OnGameInit();
        void OnGameLeave();
        void OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true) { if (m_game) m_game->OnBoxDestroy(x,y,by_bomb); };
        void OnPlayerFieldChange(uint32 oldX, uint32 oldY, uint32 newX, uint32 newY);

        bool AddBomb(uint32 x, uint32 y);
        void PreBoomBomb(uint32 x, uint32 y);

        float GetPlayerSpeedCoef() { return IsCheatOn(CHEAT_MAX_SPEED)?2.0f:m_plSpeedCoef; };
        uint32 GetFlameReach() { return m_plFlameReach; };
        uint32 GetMaxBombs() { return m_plMaxBombs; };

        void SetPlayerSpeedCoef(float value, bool relative = false) { m_plSpeedCoef = (relative)?(m_plSpeedCoef+value):(value); };
        void SetFlameReach(uint32 value, bool relative = false) { m_plFlameReach = (relative)?(m_plFlameReach+value):(value); };
        void SetMaxBombs(float value, bool relative = false) { m_plMaxBombs = (relative)?(m_plMaxBombs+value):(value); };

        void SetGameEndTime(clock_t end) { m_gameEndTime = end; };
        clock_t GetGameEndTime() { return m_gameEndTime; };

        GameTypeResources* GetGameTypeResources() { if (m_game) return &m_game->gameResources; else return NULL; };
        void AddResourceModelID(uint32 id) { if (m_game) m_game->AddEnemyModelID(id); }

        uint32 GetSetting(SettingsEnum pos) { if (pos >= SETTING_MAX) return 0; else return m_settings[pos]; };
        void SetSetting(SettingsEnum pos, uint32 val) { if (pos < SETTING_MAX) m_settings[pos] = val; };
        std::vector<uint32>* SettingsPointer() { return &m_settings; };

        bool IsDangerousField(uint32 x, uint32 y);
        bool WouldBeDangerousField(uint32 x, uint32 y);
        void SetDangerous(uint32 x, uint32 y, BombRecord* origin, clock_t since, uint32 howLong);

        void PlayerDied();
        void UpdatePlayerMoveAngle();
        void UpdatePlayerMotion();
        void SetMoveElement(uint8 direction) { if (direction < MOVE_MAX) m_moveElements[direction] = true; };
        void UnsetMoveElement(uint8 direction) { if (direction < MOVE_MAX) m_moveElements[direction] = false; };
        void BlockMovement() { m_movementBlocked = true; };
        void UnblockMovement() { m_movementBlocked = false; };
        void PauseGame();
        void UnpauseGame();
        bool IsGamePaused() { return m_gamePaused; };
        EnemyList* GetEnemies() { if (m_game) return m_game->GetEnemies(); else return NULL; };

        ModelDisplayListRecord* GetPlayerRec() { return m_playerRec; };

        // verejne pristupne, tady to nebude vadit
        PlayerStats localPlayerStats;

        bool IsConsoleOpened() { return m_console; };
        void OpenConsole() { if (IsGamePaused()) m_console = true; };
        void CloseConsole() { m_console = false; };
        const char* GetConsoleOutput(uint32 line) { if (line >= CONSOLE_OUTPUT_LINES) return ""; else return m_consoleOutput[line].c_str(); };
        const char* GetConsoleInputHistory(uint32 line) { if (line >= CONSOLE_INPUT_HISTORY) return ""; else return m_consoleInputHistory[line].c_str(); };
        void ConsoleWrite(const char* str, ...);
        void ConsoleSubmit();
        const char* GetConsoleInput() { return m_consoleInput.c_str(); };
        void SetConsoleInput(const char* inp) { m_consoleInput = inp; }
        uint32 GetCheatValue(uint32 id) { if (id >= MAX_CHEAT) return 0; else return m_cheatMap[id]; };
        bool IsCheatOn(uint32 id) { return (GetCheatValue(id) > 0); };

    private:
        std::vector<uint32> m_settings;
        GameTypeTemplate* m_game;
        std::list<BombRecord*> BombMap;
        std::vector<std::vector<std::vector<DangerousField*>>> DangerousMap;
        std::vector<bool> m_moveElements;
        clock_t m_lastMovementUpdate;
        float m_moveAngle;
        bool m_movementBlocked;
        bool m_playerDead;
        bool m_gamePaused;
        clock_t m_pauseTime;
        clock_t m_gameEndTime;
        uint8 m_stepSoundIndicator;

        bool m_console;
        uint32 m_cheatMap[MAX_CHEAT];
        std::string m_consoleOutput[CONSOLE_OUTPUT_LINES];
        std::string m_consoleInputHistory[CONSOLE_INPUT_HISTORY];
        std::string m_consoleInput;

        ModelDisplayListRecord* m_playerRec;
        uint32 m_playerX, m_playerY;

        float  m_plSpeedCoef;
        uint32 m_plFlameReach;
        uint32 m_plMaxBombs;

        uint32 m_plActiveBombs;
};

#define sGameplayMgr Singleton<GameplayMgr>::instance()

/** \fn GameplayMgr::OnBoxDestroy(uint32 x, uint32 y, bool by_bomb = true)
 *  \brief Vola se pri zniceni bedny na danych souradnicich - pouze vola metodu herniho typu
 */

/** \fn GameplayMgr::IsSingleGameType
 *  \brief Vraci true, pokud se jedna o singleplayerovy mod hry
 */

/** \fn GameplayMgr::GetPlayerSpeedCoef
 *  \brief Vraci koeficient rychlosti hrace
 */

/** \fn GameplayMgr::GetFlameReach
 *  \brief Vraci pocet poli, jez zasahne vybuch bomby
 */

/** \fn GameplayMgr::GetMaxBombs
 *  \brief Vraci pocet bomb, ktere muze mit hrac v jeden cas aktivni
 */

/** \fn GameplayMgr::SetPlayerSpeedCoef
 *  \brief Nastavuje koeficient rychlosti hrace
 */

/** \fn GameplayMgr::SetFlameReach
 *  \brief Nastavuje dosah bomby
 */

/** \fn GameplayMgr::SetMaxBombs
 *  \brief Nastavuje maximalni pocet akivnich bomb
 */

/** \fn GameplayMgr::SetGameEndTime
 *  \brief Nastavi cas konce hry
 */

/** \fn GameplayMgr::GetGameEndTime
 *  \brief Vraci cas konce probihajici hry
 */

/** \fn GameplayMgr::GetGameTypeResources
 *  \brief Vraci ukazatel na strukturu s potrebnymi vecmi do daneho typu hry
 */

 /** \fn GameplayMgr::AddResourceModelID
 *  \brief Pridava do hernich dat k nacteni ID modelu
 */

/** \fn GameplayMgr::GetSetting
 *  \brief Vraci hodnotu prvku herniho nastaveni
 */

/** \fn GameplayMgr::SetSetting
 *  \brief Nastavi prvek herniho nastaveni
 */

/** \fn GameplayMgr::SettingsPointer
 *  \brief Vraci ukazatel na vektor hernich nastaveni
 */

/** \fn GameplayMgr::SetMoveElement
 *  \brief Nastavi prvek pohybu hrace (dopredu, dozadu, doleva, doprava)
 */

/** \fn GameplayMgr::UnsetMoveElement
 *  \brief Odnastavu prvek pohybu hrace
 */

/** \fn GameplayMgr::BlockMovement
 *  \brief Zablokuje pohyb hrace (pauza, umrti, ..)
 */

/** \fn GameplayMgr::UnblockMovement
 *  \brief Povoli pohyb hraci
 */

/** \fn GameplayMgr::IsGamePaused
 *  \brief Vraci true, pokud je hra zapauzovana
 */

/** \fn GameplayMgr::GetEnemies
 *  \brief Vraci navratovou hodnotu stejnojmenne funkce herniho typu
 */

/** \fn GameplayMgr::GetPlayerRec
 *  \brief Vraci zaznam v displaylistu modelu hrace
 */

#endif
