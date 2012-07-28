#ifndef BOMB_SOUNDMGR_H
#define BOMB_SOUNDMGR_H

#include <Global.h>
#include <Singleton.h>
#include <FreeSLW.h>

/** \struct SoundEffectRecord
 *  \brief Struktura zaznamu mapy vsech prehravanych zvukovych efektu
 */
struct SoundEffectRecord
{
    uint32 sound_id;
    bool repeat;
    bool manual_remove;
    bool remove;
    clock_t start_time;
    freeslw::Sound* audiosource;
};

/** \var SoundEffectRecord::sound_id
 *  \brief ID zdrojoveho zvukoveho zaznamu
 */

/** \var SoundEffectRecord::manual_remove
 *  \brief Priznak pro rucni odebrani externe
 */

/** \var SoundEffectRecord::remove
 *  \brief Priznak Update funkci tridy SoundMgr k odebrani zvuku (zastaveni a odebrani)
 */

/** \var SoundEffectRecord::audiosource
 *  \brief Ukazatel na wrapper pro streamovani prehravaneho zvukoveho efektu
 */

typedef std::vector<uint32> PlaylistVector;

/** \class SoundMgr
 *  \brief Trida starajici se o zvuk, hudbu a veskere rutiny s tim spojene
 */
class SoundMgr
{
    public:
        SoundMgr();
        ~SoundMgr();

        void Initialize();
        void Update();

        void InitMusicPlaylist();
        void MusicStart();
        void MusicStop();
        void MusicPause();
        void MusicUnpause();
        bool IsMusicPlaying() { return m_playing; };
        int32 GetCurrentMusicId() { if (!m_playlist.empty() && m_playlistPos < m_playlist.size()) return m_playlist[m_playlistPos]; else return -1; };
        clock_t GetLastMusicStart() { return m_lastMusicStart; };

        SoundEffectRecord* PlayEffect(uint32 sound_id, bool repeat = false, bool manual_remove = false, uint32 delay_by = 0);
        void StopSoundEffect(SoundEffectRecord* rec);

        freeslw::Sound* CreateMusicSource(std::string filename);
        freeslw::Sound* CreateSoundEffectSource(std::string filename);

    private:
        freeslw::AudioInterface* m_audioMgr;
        freeslw::AudioInterface* m_audioEffectMgr;
        freeslw::Sound* m_current;
        clock_t m_lastMusicStart;

        freeslw::Sound* m_footSteps[4];

        std::list<SoundEffectRecord*> m_effectsPlayed;

        PlaylistVector m_playlist;
        uint32 m_playlistPos;
        bool m_playing;
};

#define sSoundMgr Singleton<SoundMgr>::instance()

/** \fn SoundMgr::IsMusicPlaying
 *  \brief Vraci true, pokud hraje hudba
 */

/** \fn SoundMgr::GetCurrentMusicId
 *  \brief Zjisti ID aktualne prehravane hudby
 */

#endif
