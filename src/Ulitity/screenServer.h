// Based on Bodmer's example sketch

// Reads a screen image off the TFT and send it to a processing client sketch
// over the serial port.

// This sketch has been created to work with the TFT_eSPI library here:
// https://github.com/Bodmer/TFT_eSPI

// Created by: Bodmer 27/1/17
// Updated by: Bodmer 10/3/17
// Updated by: Bodmer 23/11/18 to support SDA reads and the ESP32
// Version: 0.08

// MIT licence applies, all text above must be included in derivative works

#include <TFT_eSPI.h>

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

//====================================================================================
//                           Screen server call with no filename
//====================================================================================
// Start a screen dump server (serial or network) - no filename specified
bool screenServer(TFT_eSPI& tft);

//====================================================================================
//                           Screen server call with filename
//====================================================================================
// Start a screen dump server (serial or network) - filename specified
bool screenServer(String filename, TFT_eSPI& tft);

//====================================================================================
//                Serial server function that sends the data to the client
//====================================================================================
bool serialScreenServer(String filename, TFT_eSPI& tft);

//====================================================================================
//    Send screen size etc using a simple header with delimiters for client checks
//====================================================================================
void sendParameters(String filename, TFT_eSPI& tft);
