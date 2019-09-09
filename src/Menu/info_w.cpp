
#include "info_w.h"

/*####################################################
    Info widget
####################################################*/
info_W::info_W()
{
    #ifdef DEBUG_MODE
        Serial.println("Create info");
    #endif

    if(!list.attachComponent(&txt0)) Serial.println("Fail!");
    if(!list.attachComponent(&txt1)) Serial.println("Fail!");
    if(!list.attachComponent(&txt2)) Serial.println("Fail!");
    if(!list.attachComponent(&txt3)) Serial.println("Fail!");
    attachComponent(&list);
}

void info_W::update()
{
    label1 = String("Hola\noo\ng\nl\n") + String(millis());
    label2 = String(millis());
}
