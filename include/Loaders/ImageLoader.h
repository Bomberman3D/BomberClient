#ifndef BOMB_IMAGE_LOADER_H
#define BOMB_IMAGE_LOADER_H

#include <Global.h>

namespace Loaders
{
    void LoadGLImage(uint32* dest, const char* path);
    void LoadTexture(uint32 id);
};

#endif
