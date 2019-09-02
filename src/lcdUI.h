
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include "tftLCD.h"
#include "widgets.h"
#include "Widgets/verticalBox.h"
#include "Widgets/horizontalBox.h"
#include "Widgets/textBox.h"

class info_W final: public canvas
{
public:
    info_W();

    void update();

private:
    String label0 = "Segom";
    String label1 = "Pirmera\nSegona\n1\n2\n3\n4";
    String label2 = "Segom";
    String label3 = "Segom";
    horizontalBox<4> list = horizontalBox<4>(true);
    textBox txt0 = textBox(&label0, fillMode::BotLeft, TFT_WHITE, NULL, 1, false);
    textBox txt1 = textBox(&label1, fillMode::BotCenter, TFT_WHITE, FM12, 1);
    textBox txt2 = textBox(&label2, fillMode::BotLeft, TFT_WHITE, NULL, 3);
    textBox txt3 = textBox(&label3, fillMode::BotLeft, TFT_WHITE, NULL, 4, false);
};

class black_W final: public canvas
{
public:
    black_W(tftLCD* tft);

    void update();

private:
};

class lcdUI : public tftLCD
{
public:
    lcdUI();
    ~lcdUI();

    enum menu : uint8_t
    {
        black=0,
        info,
        main,
        settings,
        SDmenu,
        control
    };

    canvas* base = NULL;

    // Functions
    bool updateDisplay(uint8_t fps);
    bool setScreen(menu idx);
    uint32_t getUpdateTime();
    
private:
    uint8_t state = 255;
    menu menuid = menu::black;
    bool rendered = false;
    unsigned long updateTime = 0;

    canvas* updateObjects(menu id);
    /* void drawInfo(bool init);
    void drawBlack(bool init); */
};

#endif
