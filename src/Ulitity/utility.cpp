
#include "utility.h"

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    int64_t Utils::startTime;

    void Utils::tic()
    {
        startTime = esp_timer_get_time();
    }

    const int64_t Utils::toc()
    {
        int64_t eTime = esp_timer_get_time()-startTime;
        ESP_LOGD("Utility", "\nElapsed time: %lu\n", (unsigned long)eTime);
        return eTime;
    }
#endif
