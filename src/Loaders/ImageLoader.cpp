#include <Global.h>
#include <ImageLoader.h>
#include <Storage.h>
#include <SOIL.h> // Simple OpenGL Image Library hlavickovy soubor

void Loaders::LoadGLImage(uint32* dest, const char* path)
{
    if (!dest)
        dest = new uint32;

    fprintf(stdout, "Nacitani souboru %s\n", path);

    int width = 0, height = 0, channels = 0;
    GLubyte *imgArray;
    imgArray = SOIL_load_image(path, &width, &height, &channels, SOIL_LOAD_AUTO);

    glGenTextures(1, dest);
    glBindTexture(GL_TEXTURE_2D, *dest);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if(channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgArray);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imgArray);

    SOIL_free_image_data( imgArray );
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

    LoadGLImage(dest, path);

    if (dest)
        sStorage->Textures[id] = dest;
}
