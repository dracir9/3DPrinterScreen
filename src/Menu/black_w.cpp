
#include "black_w.h"

/*####################################################
    Black screen widget
####################################################*/
Black_W::Black_W(lcdUI* UI, tftLCD* tft):
    Screen(UI)
{
    tft->fillScreen(TFT_BLACK);
}

void Black_W::update(uint32_t deltaTime)
{

}

void Black_W::render(tftLCD *tft)
{

}
