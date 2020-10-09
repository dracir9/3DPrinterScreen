
#ifndef UTILITY_H
#define UTILITY_H

#include <Arduino.h>

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    static int64_t startTime;

    #define TIC  startTime = esp_timer_get_time();
    #define TOC printf("Elapsed time: %lu", (unsigned long)(esp_timer_get_time()-startTime));
#else
    #define TIC
    #define TOC
#endif

#endif
