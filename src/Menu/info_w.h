
#ifndef INFO_W_H
#define INFO_W_H

#include "widgets.h"
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
    horizontalBox<4> list = horizontalBox<4>();
    textBox txt0 = textBox(&label0, fillMode::BotLeft, TFT_WHITE, NULL, 1, false);
    textBox txt1 = textBox(&label1, fillMode::CenterLeft, TFT_WHITE, FM12, 1);
    textBox txt2 = textBox(&label2, fillMode::TopLeft, TFT_WHITE, NULL, 3);
    textBox txt3 = textBox(&label3, fillMode::CenterRight, TFT_WHITE, NULL, 4, false);
};

#endif
