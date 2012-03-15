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
    LoadMapObjectData();

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

// Prostor pro helper funkce

t3DObject* Storage::FindModelObject(uint32 modelId, const char *objectname)
{
    if (!Models[modelId] || Models[modelId]->pObject.empty())
        return NULL;

    for (std::vector<t3DObject>::iterator itr = Models[modelId]->pObject.begin(); itr != Models[modelId]->pObject.end(); ++itr)
        if (strcmp(itr->strName, objectname) == 0)
            return (&(*itr));

    return NULL;
}

ModelAnimType Storage::GetAnimTypeForFrame(uint32 modelId, uint32 frame)
{
    if (!Models[modelId] || Models[modelId]->pObject.empty())
        return ANIM_NONE;

    // preskocit "zadnou" animaci
    for (uint32 i = 1; i < MAX_ANIM; i++)
        if (ModelAnimation[modelId].Anim[i].frameFirst <= frame && ModelAnimation[modelId].Anim[i].frameLast >= frame)
            return ModelAnimType(i);

    return ANIM_NONE;
}

ObjectModifierData* Storage::GetObjectModifierData(uint32 modelId, const char* objectname)
{
    // Neexistuje
    if (ModelFilename.find(modelId) == ModelFilename.end())
        return NULL;

    IDNamePair index = std::make_pair(modelId, objectname);
    ObjectModifierMap::iterator itr = ObjectModifiers.find(index);
    if (itr == ObjectModifiers.end())
        return NULL;

    return &((*itr).second);
}
