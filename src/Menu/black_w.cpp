
#include "black_w.h"

/*####################################################
    Black screen widget
####################################################*/
black_W::black_W(tftLCD* tft)
{
    tft->fillScreen(TFT_BLACK);
}

void black_W::update()
{

}
