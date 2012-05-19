#include <Global.h>
#include <ImageLoader.h>
#include <Storage.h>
#include <SOIL.h> // Simple OpenGL Image Library hlavickovy soubor

void Loaders::LoadGLImage(uint32* dest, const char* path, uint8 flags)
{
    if (!dest)
        dest = new uint32;

    fprintf(stdout, "Nacitani souboru %s\n", path);

    uint32 soilflags = SOIL_FLAG_POWER_OF_TWO;

    if (!(flags & IMAGE_LOAD_NO_REPEAT))
        soilflags |= SOIL_FLAG_TEXTURE_REPEATS;

    if (flags & IMAGE_LOAD_MIPMAP)
        soilflags |= SOIL_FLAG_MIPMAPS;

    if (flags & IMAGE_LOAD_COMPRESS)
        soilflags |= SOIL_FLAG_COMPRESS_TO_DXT;

    (*dest) = SOIL_load_OGL_texture(path, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, soilflags);
}

void Loaders::LoadTexture(uint32 id)
{
    // Neexistuje
    if (sStorage->Textures.find(id) == sStorage->Textures.end())
        return;

    // Uz je nactena
    if (sStorage->Textures[id] != NULL)
        return;

    uint32 *dest = new uint32;
    char* path = new char[2048];
    sprintf(path, "%s/%s",DATA_PATH, sStorage->TextureFilename[id].c_str());

    fprintf(stdout, "Nacitani textury %u: %s\n", id, path);

    LoadGLImage(dest, path, sStorage->TextureInfo[id].flags);

    if (dest)
        sStorage->Textures[id] = dest;
}
