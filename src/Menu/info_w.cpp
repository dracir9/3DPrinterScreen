
#include "info_w.h"

/*####################################################
    Info widget
####################################################*/
info_W::info_W()
{
    #ifdef DEBUG_MODE
        printf("Create info\n");
    #endif

    if(!list.attachComponent(&txt0)) printf("Fail!\n");
    if(!list.attachComponent(&txt1)) printf("Fail!\n");
    if(!list.attachComponent(&txt2)) printf("Fail!\n");
    if(!list.attachComponent(&txt3)) printf("Fail!\n");
    attachToRoot(&list);
}

void info_W::update()
{
    label1 = String("Hola\noo\ng\nl\n") + String(millis());
    label2 = String(millis());
}
