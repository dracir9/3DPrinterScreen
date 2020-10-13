
#include "info_w.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "info_w"

/*####################################################
    Info widget
####################################################*/
Info_W::Info_W()
{
    ESP_LOGV(TAG, "Create info\n");

    if(!list.attachComponent(&txt0)) printf("Fail!\n");
    if(!list.attachComponent(&txt1)) printf("Fail!\n");
    if(!list.attachComponent(&txt2)) printf("Fail!\n");
    if(!list.attachComponent(&txt3)) printf("Fail!\n");
    attachToRoot(&list);
}

void Info_W::update(uint32_t deltaTime)
{
    label1 = String("Hola\noo\ng\nl\n") + String(millis());
    label2 = String(millis());
}
