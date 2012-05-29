#include <Global.h>
#include <Application.h>
#include <Stages.h>
#include <LoadingThread.h>

void IntroOutroStage::OnEnter()
{
    sDisplay->Initialize();
    sLoader->RequestLoadBlocking(LOAD_TEXTURE, 70);
    sLoader->RequestLoadBlocking(LOAD_TEXTURE, 71);

    startTime = clock();
}

void IntroOutroStage::OnDraw()
{
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    sDisplay->PrintText(FONT_ONE, 0, 40, 0.35f, TEXT_FLAG_CENTERED_X, NOCOLOR, "Martin Úbl, ve spolupráci se SPŠE Plzeò a tøídou 3.L uvádí...");
    sDisplay->Draw2D(71, WIDTH / 2 - 175, HEIGHT / 2 - 175, 350, 350);
    sDisplay->Draw2D(70, 40, HEIGHT - 140 - 40, 140, 140);

    sDisplay->Setup3DMode();

    // Automaticky prehodit do menu po 2 vterinach
    if (startTime + 2000 < clock())
        sApplication->SetStage(STAGE_MENU);
}

void IntroOutroStage::OnKeyStateChange(uint16 key, bool press)
{
    if (startTime + 1000 < clock() && press)
    {
        sApplication->SetStage(STAGE_MENU);
    }
}

void IntroOutroStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
}
