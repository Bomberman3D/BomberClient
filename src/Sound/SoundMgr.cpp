#include <Global.h>
#include <SoundMgr.h>

/** \brief Konstruktor
 *
 * Pouze nulovani veskerych hodnot a ukazatelu
 */
SoundMgr::SoundMgr()
{
    m_playlist.clear();
    m_playlistPos = 0;

    m_audioMgr = NULL;
    m_current = NULL;
    m_playing = false;
}

/** \brief Destruktor
 *
 * Uvolneni pameti, bezpecne zniceni audiomanageru
 */
SoundMgr::~SoundMgr()
{
    if (m_audioMgr)
        m_audioMgr->shutDown();

    cAudio::destroyAudioManager(m_audioMgr);
}

/** \brief Inicializace zvukoveho enginu
 *
 * Zde se vytvori audiomanager a nastavi se globalni hodnota hlasitosti
 */
void SoundMgr::Initialize()
{
    m_audioMgr = cAudio::createAudioManager(true);

    if (m_audioMgr)
        m_audioMgr->setMasterVolume(0.5f);
}

/** \brief Update funkce
 *
 * Zde je obstaravano spousteni pisnicek a funkce playlistu
 */
void SoundMgr::Update()
{
    if (!m_audioMgr)
        return;

    // Pokud nehrajeme, jen zkontrolujeme, zdali nahodou nehraje nejaky zvuk
    if (!m_playing)
    {
        // Pripadne ho zastavime
        if (m_current && m_current->isPlaying())
            m_current->stop();

        return;
    }

    if (m_playlist.empty())
        return;

    if (m_current)
    {
        // Pokud jeste muziku prehravame, neni co delat
        if (m_current->isPlaying())
            return;

        // pozice v playlistu se inkrementuje jen pokud skoncilo prehravani predchozi polozky
        m_playlistPos++;
    }

    // Preteceni playlistu
    // TODO: random prehravani
    if (m_playlistPos >= m_playlist.size())
        m_playlistPos = 0;

    if (m_playlist[m_playlistPos] >= sStorage->MusicData.size())
        return;

    // Vytvorime novy zdroj a nechame ho prehrat
    m_current = CreateMusicSource(sStorage->MusicData[m_playlist[m_playlistPos]].filename.c_str());

    if (m_current)
        m_current->play2d(false);
}

/** \brief Inicializace a sestaveni playlistu
 *
 * Prozatim se berou vsechny hudebni skladby
 */
void SoundMgr::InitMusicPlaylist()
{
    m_playlist.clear();
    m_playlistPos = 0;

    for (Storage::MusicDataMap::iterator itr = sStorage->MusicData.begin(); itr != sStorage->MusicData.end(); ++itr)
        m_playlist.push_back(itr->first);
}

/** \brief Postara se o spuseni hudby
 */
void SoundMgr::MusicStart()
{
    if (!m_audioMgr || m_playlist.empty())
        return;

    m_playing = true;

    m_current = CreateMusicSource(sStorage->MusicData[m_playlist[m_playlistPos]].filename.c_str());

    if (m_current)
        m_current->play2d(false);
}

/** \brief Postara se o zastaveni hudby
 */
void SoundMgr::MusicStop()
{
    m_playing = false;

    if (m_current)
        m_current->stop();
}

/** \brief Pauza hudby
 */
void SoundMgr::MusicPause()
{
    m_playing = false;

    if (m_current)
        m_current->pause();
}

/** \brief Odpauzovani hudby
 */
void SoundMgr::MusicUnpause()
{
    m_playing = false;

    if (m_current)
        m_current->play();
}

/** \brief Postara se o vytvoreni zdroje hudby (pro streamovani z disku)
 */
cAudio::IAudioSource* SoundMgr::CreateMusicSource(std::string filename)
{
    std::stringstream tmpname;
    tmpname.clear();
    tmpname << DATA_PATH << "/music/";
    tmpname << filename;
    tmpname << '\0';

    return m_audioMgr->create("music", tmpname.str().c_str(), true);
}
