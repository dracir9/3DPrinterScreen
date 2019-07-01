
#include "lcdUI.h"

/**************************************************************************/
/*!
    @brief  Return Top-Left coordinates of a cell (Not constrained tot the actual size of the grid)
    @param  x    Where the X coordinate will be stored
    @param  y    Where the Y coordinate will be stored
    @param  c    Column index
    @param  r    Row index
*/
/**************************************************************************/
void grid::getTLPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const
{
    *x = gridX + cellW*c + space*c;
    *y = gridY + cellH*r + space*r;
}

/**************************************************************************/
/*!
    @brief  Return center coordinates of a cell (Not constrained tot the actual size of the grid)
    @param  x    Where the X coordinate will be stored
    @param  x    Where the Y coordinate will be stored
    @param  c    Column index
    @param  r    Row index
*/
/**************************************************************************/
void grid::getCenterPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const
{
    *x = gridX + cellW*c + space*c + cellW/2;
    *y = gridY + cellH*r + space*r + cellH/2;
}

/**************************************************************************/
/*!
    @brief  Print a string centered at cursor location
    @param  String to print
*/
/**************************************************************************/
template <class text>
void lcdUI::printCenter(text s)
{
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
    getTextBounds(s, 0, 0, &x, &y, &w, &h);
    setCursor(getCursorX()-w/2, getCursorY()-h/2);
    print(s);
    
}

/**************************************************************************/
/*!
    @brief  Update the LCD display with new information
    @param  ID of the screen to be displayed/updated
*/
/**************************************************************************/
void lcdUI::updateDisplay(uint8_t id)
{
    bool init=false;
    if(id!=state) init=true;
    switch (id)
    {
        case menu::info:
            drawInfo(init);
    }

    base.render(this);
    state=id;
}

/**************************************************************************/
/*!
    @brief  Draw the info screen
    @param  Should initialize the screen?
*/
/**************************************************************************/
/* void lcdUI::drawInfo(bool init)
{
     static uint32_t cnt;
    const uint8_t e=3;

    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;

    setTextSize(2);
    getTextBounds(String("Current"), 0, 0, &x, &y, &w, &h);
    const grid temp(w+18, 40, width()-w-18, 91, e, 3, 8);
    temp.getTLPoint(&x, &y, 0, 3);
    const grid position(0, y, width(), temp.cellH, 4, 1, 8);
    position.getTLPoint(&x, &y, 0, 1);
    const grid info(0, y, width(), height()-y, 4, 2, 8);

   
    // First draw
    if (init)
    {
        //fillScreen(TFT_WHITE);
        // Header
        fillRect(0,0,HEIGHT,32,TFT_RED);
        setCursor(5,10);
        println("Hello world!");
        // End Header

        // Temperature header
        setTextColor(TFT_BLACK);
        for (uint8_t i = 0; i < e; i++)
        {
            temp.getTLPoint(&x, &y, i, 0);
            fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_WHITE);
            temp.getCenterPoint(&x, &y, i, 0);
            setCursor(x,y);
            if(i==0)
            {
                printCenter("HB");
            }
            else
            {
                printCenter("E"+String(i));
            }
        }
        // End Temperature header

        // Temperatures
        temp.getTLPoint(&x, &y, 0, 1);
        fillRoundRect(0, y, temp.gridX-temp.space, temp.cellH, 4, TFT_NAVY);
        temp.getCenterPoint(&x, &y, 0, 1);
        setCursor(5+w/2, y);
        setTextColor(TFT_WHITE);
        printCenter(String("Current"));


        temp.getTLPoint(&x, &y, 0, 2);
        fillRoundRect(0, y, temp.gridX-temp.space, temp.cellH, 4, TFT_NAVY);
        temp.getCenterPoint(&x, &y, 0, 2);
        setCursor(5+w/2, y);
        printCenter(String("Target"));
                
        temp.getTLPoint(&x, &y, 0, 0);
        drawRoundRect(0, y, width(), 92, 4, TFT_GREEN);
        // End Temperatures

        // Position
        position.getTLPoint(&x, &y, 0, 0);
        drawRoundRect(0, y, width(), position.cellH, 4, TFT_GREEN);
        info.getTLPoint(&x, &y, 0, 0);
        drawRoundRect(0, y, width(), height()-y, 4, TFT_GREEN);
        setTextSize(3);
        info.getCenterPoint(&x, &y, 0, 0);
        setCursor(x, y);
        printCenter("SD");
        setTextSize(2);
        for (uint8_t c = 0; c < 4; c++)
        {
            for (uint8_t r = 0; r < 4; r++)
            {
                info.getTLPoint(&x, &y, c, r);
                drawRect(x, y, info.cellW, info.cellH, TFT_RED);
            }
        }

        cnt=0;
    }


    // Draw update
    if (millis()%1000 == 0)
    {
        for (uint8_t i = 0; i < e; i++)
        {
            temp.getTLPoint(&x, &y, i, 1);
            fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_DARKCYAN);
            temp.getCenterPoint(&x, &y, i, 1);
            setCursor(x,y);
            printCenter(String(random(250))+String(char(247)));
            temp.getTLPoint(&x, &y, i, 2);
            fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_DARKCYAN);
            temp.getCenterPoint(&x, &y, i, 2);
            setCursor(x,y);
            printCenter(String(random(70))+String(char(247)));
        }
        
        position.getTLPoint(&x, &y, 0, 0);
        fillRoundRect(1, y+1, width()-2, position.cellH-2, 4, TFT_BLACK);
        position.getCenterPoint(&x, &y, 0, 0);
        setCursor(x,y);
        printCenter(String("X:") + String(random(250)));
        position.getCenterPoint(&x, &y, 1, 0);
        setCursor(x,y);
        printCenter(String("Y:") + String(random(210)));
        position.getCenterPoint(&x, &y, 2, 0);
        setCursor(x,y);
        printCenter(String("Z:") + String(random(200)));
        position.getCenterPoint(&x, &y, 3, 0);
        setCursor(x,y);
        printCenter(String("F_R:") + String(random(150)) + String("%"));
    }
    if (cnt != millis()/100)
    {
        cnt=millis()/100;
        setCursor(5,26);
        fillRect(5,getCursorY(),46,15,TFT_BLUE);
        print(cnt);
    } 
} */

void lcdUI::drawInfo(bool init)
{
    if(init) 
    {
        //static verticalBox list = verticalBox(3, false);
        static verticalBox list = verticalBox(3, false);
        static textBox txt1 = textBox("Pri\nmer", fillMode::CenterCenter, 8, TFT_WHITE);
        static textBox txt2 = textBox("Segon", fillMode::BotCenter, 8, TFT_WHITE);
        static textBox txt3 = textBox("Tercer", fillMode::BotCenter, 8, TFT_WHITE);
        if(list.attachComponent(&txt1)) Serial.println("Succes!");
        if(list.attachComponent(&txt2)) Serial.println("Succes!");
        if(list.attachComponent(&txt3)) Serial.println("Succes!");
        base.attachComponent(&list);
    }
}
