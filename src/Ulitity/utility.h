
#ifndef UTILITY_H
#define UTILITY_H

#define TIC int64_t startTime = esp_timer_get_time();
#define TOC printf("Elapsed time: %lu", esp_timer_get_time()-startTime);

#endif
