#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Timer.h>
#include <Effects/Animations.h>
#include <Map.h>
#include <Gameplay.h>
#include <Storage.h>

void LoadingStage::PreLoad(LoadType type, uint32 sourceId)
{
    sLoader->RequestLoad(type, sourceId);
    m_toLoad.push_back(std::make_pair(type, sourceId));
}

bool LoadingStage::IsAllLoaded()
{
    for (std::vector<LoadPair>::const_iterator itr = m_toLoad.begin(); itr != m_toLoad.end(); ++itr)
    {
        if ((*itr).first == LOAD_MODEL && sStorage->Models[(*itr).second] == NULL)
            return false;
        else if ((*itr).first == LOAD_TEXTURE && sStorage->Textures[(*itr).second] == NULL)
            return false;
    }

    return true;
}

float LoadingStage::GetLoadingPercentage()
{
    float finished = 0;

    for (std::vector<LoadPair>::const_iterator itr = m_toLoad.begin(); itr != m_toLoad.end(); ++itr)
    {
        if ((*itr).first == LOAD_MODEL && sStorage->Models[(*itr).second] != NULL)
            finished += 1.0f;
        else if ((*itr).first == LOAD_TEXTURE && sStorage->Textures[(*itr).second] != NULL)
            finished += 1.0f;
    }

    return (finished / float(m_toLoad.size()));
}

void LoadingStage::OnEnter()
{
    m_toLoad.clear();

    ImgAnimTicket = sAnimator->GetTextureAnimTicket(21, 1, 1, ANIM_FLAG_FORCE_LOADING);

    // GameSettings stage by nam mela do storage nastavit ID mapy, kterou si uzivatel vybral
    sMapManager->LoadMap(sGameplayMgr->GetSetting(SETTING_MAP_ID));

    // Nacist modely a jejich display listy!
    PreLoad(LOAD_MODEL, 8);
    PreLoad(LOAD_MODEL, 9);

    Map* pMap = (Map*)sMapManager->GetMap();
    uint32 skybox = pMap->m_skybox;

    for (uint32 i = 0; i < 6; i++)
        PreLoad(LOAD_TEXTURE, sStorage->SkyboxData[skybox].box_textures[i]);

    // Prednacist i castice vybuchu - abychom zamezili probliknuti bile textury
    PreLoad(LOAD_TEXTURE, 31);
    for (uint32 i = 57; i <= 60; i++)
        PreLoad(LOAD_TEXTURE, i);
}

void LoadingStage::OnDraw(uint32 diff)
{
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    sDisplay->PrintText(FONT_ONE, 0, HEIGHTPCT*50+100, FONT_SIZE_2, TEXT_FLAG_CENTERED_X, NOCOLOR, "Naèítání");
    sDisplay->PrintText(FONT_ONE, 0, HEIGHTPCT*50+150, FONT_SIZE_4, TEXT_FLAG_CENTERED_X, NOCOLOR, "%u%%", uint32(GetLoadingPercentage()*100.0f));

    sDisplay->Draw2D(sAnimator->GetActualTexture(ImgAnimTicket), WIDTHPCT*50-75, HEIGHTPCT*50-75,150,150);

    if (IsAllLoaded())
    {
        m_toLoad.clear();
        sApplication->SetStage(STAGE_GAME);
    }
}
