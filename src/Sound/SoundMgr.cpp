#include <Global.h>
#include <SoundMgr.h>
#include <Gameplay.h>

/** \brief Konstruktor
 *
 * Pouze nulovani veskerych hodnot a ukazatelu
 */
SoundMgr::SoundMgr()
{
    m_playlist.clear();
    m_playlistPos = 0;
    m_effectsPlayed.clear();

    m_audioMgr = NULL;
    m_audioEffectMgr = NULL;
    m_current = NULL;
    m_playing = false;

    for (uint32 i = 0; i < 4; i++)
        m_footSteps[i] = NULL;
}

/** \brief Destruktor
 *
 * Uvolneni pameti, bezpecne zniceni audiomanageru
 */
SoundMgr::~SoundMgr()
{
    if (m_audioMgr)
        m_audioMgr->shutDown();

    if (m_audioEffectMgr)
        m_audioEffectMgr->shutDown();

    cAudio::destroyAudioManager(m_audioMgr);
}

/** \brief Inicializace zvukoveho enginu
 *
 * Zde se vytvori audiomanager a nastavi se globalni hodnota hlasitosti
 */
void SoundMgr::Initialize()
{
    m_audioMgr = cAudio::createAudioManager(true);
    m_audioEffectMgr = cAudio::createAudioManager(true);

    if (m_audioMgr)
        m_audioMgr->setMasterVolume(0.2f);

    if (m_audioEffectMgr)
        m_audioEffectMgr->setMasterVolume(0.75f);
}

/** \brief Update funkce
 *
 * Zde je obstaravano spousteni pisnicek a funkce playlistu
 */
void SoundMgr::Update()
{
    if (!m_audioMgr || !m_audioEffectMgr)
        return;

    // Nejdriv projedeme cely list prehravanych zvuku a odstranime vsechny, co jiz nehrajou a nemaji nastaveny priznak pro manualni smazani
    // pripadne ty, co maji nastaveny priznak pro smazani
    for (std::list<SoundEffectRecord*>::iterator itr = m_effectsPlayed.begin(); itr != m_effectsPlayed.end(); )
    {
        if (((*itr)->manual_remove && (*itr)->remove)
            || (!(*itr)->manual_remove && !(*itr)->audiosource->isPlaying()))
        {
            (*itr)->audiosource->stop();
            m_audioEffectMgr->release((*itr)->audiosource);
            itr = m_effectsPlayed.erase(itr);
            continue;
        }
        else if (!(*itr)->audiosource)
        {
            itr = m_effectsPlayed.erase(itr);
            continue;
        }
        else
            ++itr;
    }

    // Pokud nehrajeme, jen zkontrolujeme, zdali nahodou nehraje nejaky zvuk
    if (!m_playing)
    {
        // Pripadne ho zastavime
        if (m_current && m_current->isPlaying())
        {
            m_current->stop();
            m_audioMgr->release(m_current);
        }

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

    // Preteceni playlistu, vybereme nahodne dalsi seznam
    if (m_playlistPos >= m_playlist.size())
        InitMusicPlaylist();

    // Pokud se playlist znovu nevygeneroval, nemuzeme prehravat
    if (m_playlist.empty())
    {
        m_playing = false;
        return;
    }

    if (m_playlist[m_playlistPos] >= sStorage->MusicData.size())
        return;

    // Uvolnime stary zaznam
    if (m_current)
        m_audioMgr->release(m_current);

    // Vytvorime novy zdroj a nechame ho prehrat
    m_current = CreateMusicSource(sStorage->MusicData[m_playlist[m_playlistPos]].filename.c_str());
    m_lastMusicStart = clock();

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

    // Zkopirujeme si seznam hudby pro danou fazi
    std::vector<uint32> capableMusic;
    for (std::vector<uint32>::iterator itr = sGameplayMgr->GetGameTypeResources()->MusicIDs.begin(); itr != sGameplayMgr->GetGameTypeResources()->MusicIDs.end(); ++itr)
        capableMusic.push_back(*itr);

    // A pak si je nahodne vybereme a nasazime do seznamu a z puvodniho seznamu je vymazeme
    uint32 index;
    while (capableMusic.size() > 0)
    {
        index = rand()%(capableMusic.size());
        m_playlist.push_back(capableMusic[index]);

        capableMusic.erase(capableMusic.begin() + index);
    }
}

/** \brief Postara se o spuseni hudby
 */
void SoundMgr::MusicStart()
{
    if (!m_audioMgr || m_playlist.empty())
        return;

    m_playing = true;

    m_current = CreateMusicSource(sStorage->MusicData[m_playlist[m_playlistPos]].filename.c_str());
    m_lastMusicStart = clock();

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
    m_playing = true;

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

/** \brief Postara se o vytvoreni zdroje zvukoveho efektu (pro streamovani z disku)
 */
cAudio::IAudioSource* SoundMgr::CreateSoundEffectSource(std::string filename)
{
    std::stringstream tmpname;
    tmpname.clear();
    tmpname << DATA_PATH << "/sounds/";
    tmpname << filename;
    tmpname << '\0';

    return m_audioEffectMgr->create("bling", tmpname.str().c_str(), true);
}

/** \brief Vytvori zaznam pro prehravani zvukoveho efektu a spusti jeho prehravani
 */
SoundEffectRecord* SoundMgr::PlayEffect(uint32 sound_id, bool repeat, bool manual_remove)
{
    if (sStorage->SoundEffectData.find(sound_id) == sStorage->SoundEffectData.end())
        return NULL;

    // Uchovame si ukazatele na kroky postavy
    if (sound_id >= 18 && sound_id <= 21)
    {
        if (!m_footSteps[sound_id-18])
            m_footSteps[sound_id-18] = CreateSoundEffectSource(sStorage->SoundEffectData[sound_id].c_str());

        if (m_footSteps[sound_id-18])
            m_footSteps[sound_id-18]->play2d();

        return NULL;
    }

    SoundEffectRecord* pNew = new SoundEffectRecord;
    pNew->sound_id = sound_id;
    pNew->manual_remove = manual_remove;
    pNew->remove = false;
    pNew->audiosource = CreateSoundEffectSource(sStorage->SoundEffectData[sound_id].c_str());

    if (!pNew->audiosource)
    {
        delete pNew;
        return NULL;
    }

    pNew->audiosource->play2d(repeat);

    m_effectsPlayed.push_back(pNew);

    // Je bezpecnejsi pro nas vracet NULL pointer, protoze pri jakemkoliv pokusu o odstraneni by mel byt not-null check
    // bohuzel se smazanou pameti by doslo k padu aplikace, protoze vraceny pointer by ukazoval stale na to misto do pameti,
    // ktere uz nemusi byt alokovane nasi aplikaci
    if (manual_remove)
        return pNew;
    else
        return NULL;
}

/** \brief Pokud existuje, zastavi zvukovy zaznam a vymaze ho z listu prehravanych
 */
void SoundMgr::StopSoundEffect(SoundEffectRecord *rec)
{
    if (!rec)
        return;

    // Overeni, zdali je vubec prehravany nami
    // Pomuze i k overeni validity ukazatele, protoze delete muze probehnout jen a pouze tridou SoundMgr
    for (std::list<SoundEffectRecord*>::iterator itr = m_effectsPlayed.begin(); itr != m_effectsPlayed.end(); ++itr)
    {
        if ((*itr) == rec)
        {
            if (rec->audiosource)
                rec->audiosource->stop();

            rec->remove = true;
            break;
        }
    }
}
