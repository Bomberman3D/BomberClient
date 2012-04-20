#include <Global.h>
#include <SoundMgr.h>

SoundMgr::SoundMgr()
{
    m_playlist.clear();
    m_playlistPos = 0;
}

SoundMgr::~SoundMgr()
{
    if (m_audioMgr)
        m_audioMgr->shutDown();

    cAudio::destroyAudioManager(m_audioMgr);
}

void SoundMgr::Initialize()
{
    m_audioMgr = cAudio::createAudioManager(true);
    m_audioMgr->setMasterVolume(0.5f);
}

void SoundMgr::Update()
{
}

void SoundMgr::InitMusicPlaylist()
{
    m_playlist.clear();
    m_playlistPos = 0;

    for (Storage::MusicDataMap::iterator itr = sStorage->MusicData.begin(); itr != sStorage->MusicData.end(); ++itr)
        m_playlist.push_back(itr->first);
}

void SoundMgr::MusicStart()
{
    if (!m_audioMgr || m_playlist.empty())
        return;

    m_playing = true;

    std::stringstream tmpname;
    tmpname.clear();
    tmpname << DATA_PATH << "/music/";
    tmpname << sStorage->MusicData[m_playlist[m_playlistPos]].filename.c_str();
    tmpname << '\0';

    m_current = m_audioMgr->create("music", tmpname.str().c_str(), true);

    if (m_current)
        m_current->play2d(false);
}

void SoundMgr::MusicStop()
{
    if (m_current)
        m_current->stop();
}

void SoundMgr::MusicPause()
{
    if (m_current)
        m_current->pause();
}
