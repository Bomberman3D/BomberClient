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
        bool IsMusicPlaying() { return m_playing; };

    private:
        cAudio::IAudioManager* m_audioMgr;
        cAudio::IAudioSource* m_current;

        PlaylistVector m_playlist;
        uint32 m_playlistPos;
        bool m_playing;
};

#define sSoundMgr Singleton<SoundMgr>::instance()

#endif
