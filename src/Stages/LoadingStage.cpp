#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <Display.h>
#include <Timer.h>
#include <Effects/Animations.h>
#include <Map.h>
#include <Gameplay.h>
#include <LoadingThread.h>

void GameStageSet(uint32 param1, uint32 param2, uint32 param3)
{
    sApplication->SetStage(STAGE_GAME);
}

void LoadingStage::OnEnter()
{
    // TODO: nacitaci vlakno, pote nechat zde odeslat pozadavek na nacteni a vpustit do hry
    //       az po nacteni vseho potrebneho

    // Pro efekt :-D
    sTimer->AddTimedEvent(1000, &GameStageSet, 0, 0, 0);

    ImgAnimTicket = sAnimator->GetTextureAnimTicket(21, 1);

    // GameSettings stage by nam mela do storage nastavit ID mapy, kterou si uzivatel vybral
    sMapManager->LoadMap(sGameplayMgr->GetSetting(SETTING_MAP_ID));

    // Nacist modely a jejich display listy!
    sLoader->RequestLoad(LOAD_MODEL, 8);
    //Loaders::LoadModel(8); // bomba
    sDisplay->DrawModel(8, 0,0,0,ANIM_NONE, 1,0,true, false, 0, 0, ANIM_RESTRICTION_NONE, true);

    sLoader->RequestLoad(LOAD_MODEL, 9);
    //Loaders::LoadModel(9); // model postavy
    sDisplay->DrawModel(9, 0,0,0,ANIM_NONE, 1,0,true, false, 0, 0, ANIM_RESTRICTION_NONE, true);
}

void LoadingStage::OnDraw(uint32 diff)
{
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    sDisplay->PrintText(MAIN_FONT, WIDTHPCT*50-11*16/4, HEIGHTPCT*50+100,FONT_SIZE_N, NOCOLOR, "Naèítání");

    sDisplay->Draw2D(sAnimator->GetActualTexture(ImgAnimTicket), WIDTHPCT*50-75, HEIGHTPCT*50-75,150,150);
}
