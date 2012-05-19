#ifndef BOMB_IMAGE_LOADER_H
#define BOMB_IMAGE_LOADER_H

#include <Global.h>

enum LoadFlags
{
    IMAGE_LOAD_NO_REPEAT = 0x01,
    IMAGE_LOAD_COMPRESS  = 0x02,
    IMAGE_LOAD_MIPMAP    = 0x04,
};

namespace Loaders
{
    void LoadGLImage(uint32* dest, const char* path, uint8 flags = 0);
    void LoadTexture(uint32 id);
};

#endif
