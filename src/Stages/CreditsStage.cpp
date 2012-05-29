#include <Global.h>
#include <Application.h>
#include <Stages.h>

void CreditsStage::OnEnter()
{
    // Cerne pozadi
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    m_scrollPos = 0;

    // Automaticky scrolling zacne za 1s
    m_moveTimer = clock()+1000;
}

void CreditsStage::OnDraw()
{
    if (!sDisplay->IsIn2DMode())
        sDisplay->Setup2DMode();

    // Automaticky posun
    // o 2 body za 40ms (0.04s) = 50 bodu za sekundu
    if (m_moveTimer < clock())
    {
        m_scrollPos += 2;
        m_moveTimer = clock()+40;
    }

    // Limitace na maximalni bod posunu
    if (m_scrollPos > 440+400+120)
        m_scrollPos = 440+400+120;

    sDisplay->PrintText(FONT_ONE, 0, 80-m_scrollPos, FONT_SIZE_1, TEXT_FLAG_CENTERED_X, NOCOLOR, "Bomberman 3D");

    sDisplay->PrintText(FONT_ONE, 0, 170-m_scrollPos, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 127), "Vedoucí projektu:");
    sDisplay->PrintText(FONT_ONE, 0, 200-m_scrollPos, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Martin Úbl");

    sDisplay->PrintText(FONT_ONE, 0, 170-m_scrollPos+80, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 127), "Programoval:");
    sDisplay->PrintText(FONT_ONE, 0, 200-m_scrollPos+80, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Martin Úbl");

    sDisplay->PrintText(FONT_ONE, 0, 170-m_scrollPos+160, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 127), "Grafika:");
    sDisplay->PrintText(FONT_ONE, 0, 200-m_scrollPos+160, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Martin Úbl");
    sDisplay->PrintText(FONT_ONE, 0, 230-m_scrollPos+160, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "S7");
    sDisplay->PrintText(FONT_ONE, 0, 260-m_scrollPos+160, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Pipin");

    sDisplay->PrintText(FONT_ONE, 0, 170-m_scrollPos+240+60, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 127), "Hlas:");
    sDisplay->PrintText(FONT_ONE, 0, 200-m_scrollPos+240+60, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Adam Barák");

    sDisplay->PrintText(FONT_ONE, 0, 170-m_scrollPos+320+60, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 127), "Hudba:");
    sDisplay->PrintText(FONT_ONE, 0, 200-m_scrollPos+320+60, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "1024mb");
    sDisplay->PrintText(FONT_ONE, 0, 230-m_scrollPos+320+60, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Mirrored Theory");
    sDisplay->PrintText(FONT_ONE, 0, 260-m_scrollPos+320+60, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Sinetrium");

    sDisplay->PrintText(FONT_ONE, 0, 170-m_scrollPos+400+120, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 127), "Zvláštní podìkování:");
    sDisplay->PrintText(FONT_ONE, 0, 200-m_scrollPos+400+120, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Tomáš Kleisner");
    sDisplay->PrintText(FONT_ONE, 0, 230-m_scrollPos+400+120, FONT_SIZE_4, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 0), "(nápad, poskytnutí zkušebny pro nahrávání zvukù)");
    sDisplay->PrintText(FONT_ONE, 0, 260-m_scrollPos+400+120, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Freghar");
    sDisplay->PrintText(FONT_ONE, 0, 290-m_scrollPos+400+120, FONT_SIZE_4, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 0), "(pomoc pøi optimalizaci a algoritmizaci)");
    sDisplay->PrintText(FONT_ONE, 0, 320-m_scrollPos+400+120, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Jakub Horák");
    sDisplay->PrintText(FONT_ONE, 0, 350-m_scrollPos+400+120, FONT_SIZE_4, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 0), "(testování)");
    sDisplay->PrintText(FONT_ONE, 0, 380-m_scrollPos+400+120, FONT_SIZE_3, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 255), "Martin Tomášek");
    sDisplay->PrintText(FONT_ONE, 0, 410-m_scrollPos+400+120, FONT_SIZE_4, TEXT_FLAG_CENTERED_X, COLOR(255, 127, 0), "(pomoc pøi ladìní vzhledu a hratelnosti)");

    // Logo skoly - jakmile bude uprostred pohledu, zastavi se skrolovani
    sDisplay->Draw2D(70, (WIDTH/2)-70, 440+400+120-m_scrollPos+HEIGHT/2-70, 140, 140);
}

void CreditsStage::OnKeyStateChange(uint16 key, bool press)
{
    if (!press)
        return;

    // Posouvani pomoci klaves sipek
    if (key == VK_UP)
    {
        if (m_scrollPos > 20)
            m_scrollPos -= 20;
        else
            m_scrollPos = 0;

        // + pozastaveni skrolovani na 3 sekundy
        m_moveTimer = clock() + 3000;
    }
    else if (key == VK_DOWN)
    {
        m_scrollPos += 20;

        m_moveTimer = clock() + 3000;
    }
    // Jakakoliv jina klavesa vrati zpet do menu
    else
        sApplication->SetStage(STAGE_MENU);
}

void CreditsStage::OnMouseButtonPress(uint32 x, uint32 y, bool left)
{
}
