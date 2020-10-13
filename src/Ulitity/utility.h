
#ifndef UTILITY_H
#define UTILITY_H

#include <Arduino.h>

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    class Utils
    {
    public:
        //******************************************************************
        // tic()
        //******************************************************************
        // Set time mark
        //------------------------------------------------------------------
        static void tic();
        
        //******************************************************************
        // tic()
        //******************************************************************
        // Return and print time elapsed since last tic() call in microsenconds
        //------------------------------------------------------------------
        static const int64_t toc();

    private:
        static int64_t startTime;
    };

    #define TIC Utils::tic();
    #define TOC Utils:toc();
#else
    #define TIC
    #define TOC
#endif

#endif
