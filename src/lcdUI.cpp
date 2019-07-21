
#include "lcdUI.h"

/**************************************************************************/
/*!
    @brief  Update the LCD display with new information
    @param  ID of the screen to be displayed/updated
*/
/**************************************************************************/
bool lcdUI::updateDisplay(uint8_t fps)
{
    if (millis() % (1000/fps) == 0 && !rendered)
    {
        bool init=false;
        if(menuid != state)
        {
            init=true;
            base.clear();
        }
        updateObjects(menuid, init);
        base.render(this);
        state=menuid;
        rendered = true;
        return true;
    }
    else if(millis() % (1000/fps) != 0 && rendered)
    {
        rendered = false;
    }
    return false;
}

bool lcdUI::setScreen(menu idx)
{
    if (menuid != idx)
    {
        menuid = idx;
        return true;
    }
    return false;
}

bool lcdUI::updateObjects(menu id, bool init)
{
    switch (id)
    {
        case menu::black:
            drawBlack(init);
            break;
        case menu::info:
            drawInfo(init);
            break;
        case menu::main:
            break;
        case menu::SDmenu:
            break;
        case menu::settings:
            break;
        case menu::control:
            break;
    }
    return true;
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
    static String label0 = "Segom";
    static String label1 = "Pirmera\nSegona\n1\n2\n3\n4";
    static String label2 = "Segom";
    static String label3 = "Segom";
    if(init) 
    {
        verticalBox *list = new verticalBox(4, true);
        textBox *txt0 = new textBox(&label0, fillMode::BotLeft, 8, TFT_WHITE, NULL, 1, false);
        textBox *txt1 = new textBox(&label1, fillMode::BotCenter, 8, TFT_WHITE, FM12, 1);
        textBox *txt2 = new textBox(&label2, fillMode::BotLeft, 8, TFT_WHITE, NULL, 3);
        textBox *txt3 = new textBox(&label3, fillMode::BotLeft, 8, TFT_WHITE, NULL, 4, false);
        if(!list->attachComponent(txt0)) Serial.println("Fail!");
        if(!list->attachComponent(txt1)) Serial.println("Fail!");
        if(!list->attachComponent(txt2)) Serial.println("Fail!");
        if(!list->attachComponent(txt3)) Serial.println("Fail!");
        base.attachComponent(list);
        setTextColor(TFT_WHITE, TFT_BLUE);
        setTextSize(3);
        setCursor(50,50);
        setFreeFont(FM12);
        vector2<int16_t> vec(230,5);
        unsigned long start = micros();
        drawCharBg(vector2<int16_t>(240,50), 'H', TFT_WHITE, TFT_RED, 3, &vec, vector2<int16_t>(64, 50)); //2799 us
        Serial.print(micros()-start);
        Serial.println("us <-Time");
        //printBg("kokogege", vector2<int16_t>(50,5), vector2<uint16_t>(100,40));
        drawRect(50,5,100,40,TFT_RED);
    }

    label1 = String("Hola\noo\ng\nl\n") + String(millis());
    label2 = String(millis());
}

void lcdUI::drawBlack(bool init)
{
    if (init)
    {
        fillScreen(TFT_BLACK);
    }
}
