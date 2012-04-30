#ifndef BOMB_SOUNDMGR_H
#define BOMB_SOUNDMGR_H

#include <Global.h>
#include <Singleton.h>
#include <cAudio.h>

#if defined(CAUDIO_PLATFORM_WIN)
  #define AUDIO_FILE(_soundName_) CAUDIO_MEDIA_ROOT##_soundName_
#else
  #define AUDIO_FILE(_soundName_) CAUDIO_MEDIA_ROOT#_soundName_
#endif

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

        cAudio::IAudioSource* CreateMusicSource(std::string filename);

    private:
        cAudio::IAudioManager* m_audioMgr;
        cAudio::IAudioSource* m_current;

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
