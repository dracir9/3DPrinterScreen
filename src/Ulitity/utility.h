
#ifndef UTILITY_H
#define UTILITY_H

extern unsigned long startTime;

#define TIC startTime = micros();
#define TOC Serial.print("Elapsed time: "); Serial.println(micros()-startTime);

#endif
