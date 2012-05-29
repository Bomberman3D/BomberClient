#ifndef BOMB_CONFIG_H
#define BOMB_CONFIG_H

#include <Global.h>
#include <Singleton.h>

/** \class Config
 *  \brief Trida starajici se o externi nastaveni zakladnich parametru aplikace
 */
class Config
{
    public:
        Config()
        {
            // Defaultni hodnoty
            WindowWidth  = 640;
            WindowHeight = 480;
            ColorDepth   = 32;
            fullscreen   = false;
            RefreshRate  = 60;
            MusicVolume  = 30;
            EffectVolume = 65;

            HostName = "127.0.0.1";
            NetworkPort = 2530;
        }

        bool Load();
        void Save();

        uint32 WindowWidth;
        uint32 WindowHeight;
        uint8  ColorDepth;
        bool   fullscreen;
        uint32 RefreshRate;
        uint32 MusicVolume;
        uint32 EffectVolume;

        std::string HostName;
        uint32 NetworkPort;
};

#define sConfig Singleton<Config>::instance()

/** \fn Config::Config
 *  \brief Konstruktor
 *
 * Nastaveni implicitnich hodnot
 */

/** \fn Config::Load
 *  \brief Nacteni configu z defaultnich souboru
 */

/** \fn Config::Save
 *  \brief Ulozeni configu do souboru
 */

/** \var Config::WindowWidth
 *  \brief Sirka okna
 */

/** \var Config::WindowHeight
 *  \brief Vyska okna
 */

/** \var Config::ColorDepth
 *  \brief Barevna hloubka
 */

/** \var Config::fullscreen
 *  \brief Priznak pro beh v rezimu cele obrazovky
 */

/** \var Config::RefreshRate
 *  \brief Pokud se jedna o celoobrazovkovy mod, tohle je obnovovaci frekvence
 */

/** \var Config::HostName
 *  \brief IP adresa / host name sitoveho herniho serveru
 */

/** \var Config::NetworkPort
 *  \brief Port pro komunikaci se serverovou casti aplikace
 */

/** \var Config::MusicVolume
 *  \brief Hlasitost hudby v procentech
 */

/** \var Config::EffectVolume
 *  \brief Hlasitost efektu v procentech
 */

#endif
