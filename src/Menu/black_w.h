
#ifndef BLACK_W_H
#define BLACK_W_H

#include "widgets.h"

class Black_W final: public Screen
{
public:
    Black_W(tftLCD* tft);

    void update(uint32_t deltaTime);

private:
};

#endif
