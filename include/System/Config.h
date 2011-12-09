#ifndef BOMB_CONFIG_H
#define BOMB_CONFIG_H

#include <Global.h>
#include <Singleton.h>

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

            HostName = "127.0.0.1";
            NetworkPort = 2530;
        }

        bool Load();

        uint32 WindowWidth;
        uint32 WindowHeight;
        uint8  ColorDepth;
        bool   fullscreen;
        uint32 RefreshRate;

        std::string HostName;
        uint32 NetworkPort;
};

#define sConfig Singleton<Config>::instance()

#endif
