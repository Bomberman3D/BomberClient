#include <Global.h>
#include <Storage.h>

Storage::Storage()
{
}

void Storage::Load()
{
    LoadMapData();
    LoadTextureData();
    LoadModelData();
    LoadSkyboxData();

    PrepareDynamicStorages();
}

void Storage::PrepareDynamicStorages()
{
    // Vsechny ukazatele na ID textur vyNULLovat, kdyz je pointer NULL, znamena to, ze textura neni nactena
    for (IdFilenameMap::const_iterator itr = TextureFilename.begin(); itr != TextureFilename.end(); ++itr)
        Textures[itr->first] = NULL;

    MaterialTextures.clear();

    for (IdFilenameMap::const_iterator itr = ModelFilename.begin(); itr != ModelFilename.end(); ++itr)
        Models[itr->first] = NULL;
}
