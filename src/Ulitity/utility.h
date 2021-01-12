
#ifndef UTILITY_H
#define UTILITY_H

#ifdef TIMER_U
    #include <Arduino.h>
    class Timer
    {
    public:

        /**
         * @brief Sets time mark
         */
        static void tic();
        
        /**
         * @brief Print time elapsed since last tic() call in microseconds
         * 
         * @return const int64_t Time elapsed in microseconds
         */
        static const int64_t toc();

    private:
        static int64_t startTime;
    };

    #define TIC Timer::tic();
    #define TOC Timer::toc();
#else
    #define TIC
    #define TOC
#endif

#ifdef SCREEN_SERVER_U
    #include <TFT_eSPI.h>
    // Based on Bodmer's example sketch

    // Reads a screen image off the TFT and send it to a processing client sketch
    // over the serial port.

    // This sketch has been created to work with the TFT_eSPI library here:
    // https://github.com/Bodmer/TFT_eSPI

    // Created by: Bodmer 27/1/17
    // Updated by: Bodmer 10/3/17
    // Updated by: Bodmer 23/11/18 to support SDA reads and the ESP32
    // Updated by: dracir9 12/1/21 adapted to own project
    // Version: 0.08

    // MIT licence applies, all text above must be included in derivative works
    //====================================================================================
    //                                  Definitions
    //====================================================================================
    #define PIXEL_TIMEOUT 100     // 100ms Time-out between pixel requests
    #define START_TIMEOUT 10000   // 10s Maximum time to wait at start transfer

    #define BITS_PER_PIXEL 16     // 24 for RGB colour format, 16 for 565 colour format

    // File names must be alpha-numeric characters (0-9, a-z, A-Z) or "/" underscore "_"
    // other ascii characters are stripped out by client, including / generates
    // sub-directories
    #define DEFAULT_FILENAME "tft_screenshots/screenshot" // In case none is specified
    #define FILE_TYPE "png"       // jpg, bmp, png, tif are valid

    // Filename extension
    // '#' = add incrementing number, '@' = add timestamp, '%' add millis() timestamp,
    // '*' = add nothing
    // '@' and '%' will generate new unique filenames, so beware of cluttering up your
    // hard drive with lots of images! The PC client sketch is set to limit the number of
    // saved images to 1000 and will then prompt for a restart.
    #define FILE_EXT  '@'         

    // Number of pixels to send in a burst (minimum of 1), no benefit above 8
    // NPIXELS values and render times:
    // NPIXELS 1 = use readPixel() = >5s and 16 bit pixels only
    // NPIXELS >1 using rectRead() 2 = 1.75s, 4 = 1.68s, 8 = 1.67s
    #define NPIXELS 8  // Must be integer division of both TFT width and TFT height

    class ScreenServer
    {
    public:
        /**
         * @brief Start a screen dump server - no filename specified
         * 
         * @param tft TFT_eSPI instance to print
         * @return true if image is sent properly otherwise it returns false 
         */
        static bool screenServer(TFT_eSPI& tft);

        /**
         * @brief Start a screen dump server - filename specified
         * 
         * @param filename Name of the saved image
         * @param tft TFT_eSPI instance to print
         * @return true if image is sent properly otherwise it returns false 
         */
        static bool screenServer(String filename, TFT_eSPI& tft);

        /**
         * @brief  Send screen size etc using a simple header with delimiters for client checks
         * 
         * @param filename Name of the saved image
         * @param tft TFT_eSPI instance to print
         */
        static void sendParameters(String filename, TFT_eSPI& tft);
    };

    #define PRINT_SCR(tft) ScreenServer::screenServer(tft);
#else
    #define PRINT_SCR(tft)
#endif

#endif
