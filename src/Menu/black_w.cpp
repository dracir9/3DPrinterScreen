
#include "black_w.h"

/*####################################################
    Black screen widget
####################################################*/
Black_W::Black_W(tftLCD* tft)
{
    tft->fillScreen(TFT_BLACK);
}

void Black_W::update(uint32_t deltaTime)
{

}
