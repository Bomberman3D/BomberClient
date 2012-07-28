#include <Global.h>
#include <SoundMgr.h>
#include <Gameplay.h>

void soundEngineErrorCallback(const char* msg)
{
    fprintf(stdout, "SoundMgr: an error occured: %s", msg);
}

/** \brief Konstruktor
 *
 * Pouze nulovani veskerych hodnot a ukazatelu
 */
SoundMgr::SoundMgr()
{
    m_playlist.clear();
    m_playlistPos = 0;
    m_effectsPlayed.clear();
    m_lastMusicStart = 0;

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
    freeslw::ReleaseInterface();
}

/** \brief Inicializace zvukoveho enginu
 *
 * Zde se vytvori audiomanager a nastavi se globalni hodnota hlasitosti
 */
void SoundMgr::Initialize()
{
    freeslw::SetErrorCallback(soundEngineErrorCallback);

    m_audioMgr = freeslw::GetInterface(freeslw::AS_OPENAL);
    m_audioEffectMgr = freeslw::GetInterface(freeslw::AS_OPENAL);

    if (m_audioMgr)
        m_audioMgr->SetVolume((float)sConfig->MusicVolume/100.0f);

    if (m_audioEffectMgr)
        m_audioEffectMgr->SetVolume((float)sConfig->EffectVolume/100.0f);
}

/** \brief Update funkce
 *
 * Zde je obstaravano spousteni pisnicek a funkce playlistu
 */
void SoundMgr::Update()
{
    if (!m_audioMgr || !m_audioEffectMgr)
        return;

    m_audioMgr->Update();
    m_audioEffectMgr->Update();

    // Nejdriv projedeme cely list prehravanych zvuku a odstranime vsechny, co jiz nehrajou a nemaji nastaveny priznak pro manualni smazani
    // pripadne ty, co maji nastaveny priznak pro smazani
    for (std::list<SoundEffectRecord*>::iterator itr = m_effectsPlayed.begin(); itr != m_effectsPlayed.end(); )
    {
        if ((*itr)->start_time > 0 && (*itr)->start_time <= clock())
        {
            (*itr)->start_time = 0;
            if ((*itr)->audiosource)
                (*itr)->audiosource->Play();
        }

        if ((*itr)->repeat && (!(*itr)->remove))
            (*itr)->audiosource->Play();

        if (((*itr)->manual_remove && (*itr)->remove)
            || (!(*itr)->manual_remove && !(*itr)->audiosource->IsPlaying() && (*itr)->start_time == 0))
        {
            (*itr)->audiosource->Stop();
            m_audioEffectMgr->ReleaseSound((*itr)->audiosource);
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
        if (m_current && m_current->IsPlaying())
        {
            m_current->Stop();
            m_audioMgr->ReleaseSound(m_current);
        }

        return;
    }

    if (m_playlist.empty())
        return;

    if (m_current)
    {
        // Pokud jeste muziku prehravame, neni co delat
        if (m_current->IsPlaying())
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
        m_audioMgr->ReleaseSound(m_current);

    // Vytvorime novy zdroj a nechame ho prehrat
    m_current = CreateMusicSource(sStorage->MusicData[m_playlist[m_playlistPos]].filename.c_str());

    if (m_current)
    {
        m_current->Play();
        m_lastMusicStart = clock();
    }
    else
        m_lastMusicStart = 0;
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

    if (m_current)
    {
        m_current->Play();
        m_lastMusicStart = clock();
    }
    else
        m_lastMusicStart = 0;
}

/** \brief Postara se o zastaveni hudby
 */
void SoundMgr::MusicStop()
{
    m_playing = false;

    if (m_current)
        m_current->Stop();
}

/** \brief Pauza hudby
 */
void SoundMgr::MusicPause()
{
    m_playing = false;

    if (m_current)
        m_current->Pause();
}

/** \brief Odpauzovani hudby
 */
void SoundMgr::MusicUnpause()
{
    m_playing = true;

    if (m_current)
        m_current->Play();
}

/** \brief Postara se o vytvoreni zdroje hudby (pro streamovani z disku)
 */
freeslw::Sound* SoundMgr::CreateMusicSource(std::string filename)
{
    std::stringstream tmpname;
    tmpname.clear();
    tmpname << DATA_PATH << "/music/";
    tmpname << filename;
    tmpname << '\0';

    return m_audioMgr->LoadSound(tmpname.str().c_str());
}

/** \brief Postara se o vytvoreni zdroje zvukoveho efektu (pro streamovani z disku)
 */
freeslw::Sound* SoundMgr::CreateSoundEffectSource(std::string filename)
{
    std::stringstream tmpname;
    tmpname.clear();
    tmpname << DATA_PATH << "/sounds/";
    tmpname << filename;
    tmpname << '\0';

    return m_audioEffectMgr->LoadSound(tmpname.str().c_str());
}

/** \brief Vytvori zaznam pro prehravani zvukoveho efektu a spusti jeho prehravani
 */
SoundEffectRecord* SoundMgr::PlayEffect(uint32 sound_id, bool repeat, bool manual_remove, uint32 delay_by)
{
    if (sStorage->SoundEffectData.find(sound_id) == sStorage->SoundEffectData.end())
        return NULL;

    // Uchovame si ukazatele na kroky postavy
    if (sound_id >= 18 && sound_id <= 21)
    {
        if (!m_footSteps[sound_id-18])
            m_footSteps[sound_id-18] = CreateSoundEffectSource(sStorage->SoundEffectData[sound_id].c_str());

        if (m_footSteps[sound_id-18])
            m_footSteps[sound_id-18]->Play();

        return NULL;
    }

    SoundEffectRecord* pNew = new SoundEffectRecord;
    pNew->sound_id = sound_id;
    pNew->repeat = repeat;
    pNew->manual_remove = manual_remove;
    pNew->remove = false;

    if (delay_by > 0)
        pNew->start_time = clock() + delay_by;
    else
        pNew->start_time = 0;

    pNew->audiosource = CreateSoundEffectSource(sStorage->SoundEffectData[sound_id].c_str());

    if (!pNew->audiosource)
    {
        delete pNew;
        return NULL;
    }

    if (delay_by == 0)
        pNew->audiosource->Play();

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
                rec->audiosource->Stop();

            rec->remove = true;
            break;
        }
    }
}
