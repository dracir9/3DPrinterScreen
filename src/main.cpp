/*
  Have a FAT Formatted SD Card connected to the SPI port of the ESP32
  The web root is the SD Card root folder
  File extensions with more than 3 charecters are not supported by the SD Library
  File Names longer than 8 charecters will be truncated by the SD library, so keep filenames shorter
  index.htm is the default index (works on subfolders as well)

  upload the contents of SdRoot to the root of the SDcard and access the editor by going to http://esp32sd.local/edit

*/

#include "main.h"

//############################################################
//  CONFIGURATION
//############################################################

// Touch Screen
#define MINPRESSURE 200
#define MAXPRESSURE 6400
#define TOUCH_GRID_X 3
#define TOUCH_GRID_Y 4
const int XP = 27, XM = 15, YP = 4, YM = 14;
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
uint16_t brushColor = YELLOW;

//###########################################################
//  GLOBAL VARIALBES
//###########################################################

lcdUI tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//###########################################################
//  FUNCTIONS
//###########################################################

bool touchMapXY(uint16_t *x, uint16_t *y)
{
  //-X  +X
  //-Y  +Y
  static const uint16_t mappingXread[TOUCH_GRID_X] = {3700, 1900, 600};
  static const uint16_t mappingXpos[TOUCH_GRID_X] = {0, 200, 320};
  static const uint16_t mappingYread[TOUCH_GRID_Y] = {4000, 2900, 1200, 400};
  static const uint16_t mappingYpos[TOUCH_GRID_Y] = {0, 160, 400, 480};
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);   //because TFT control pins
  digitalWrite(XM, HIGH);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    for(uint8_t i = 0; i < TOUCH_GRID_X-1; i++)
    {
      if(p.x > mappingXread[i+1])
      {
        *y=map(p.x, mappingXread[i], mappingXread[i+1], mappingXpos[i], mappingXpos[i+1]);
        break;
      }
    }

    for(uint8_t i = 0; i < TOUCH_GRID_Y-1; i++)
    {
      if(p.y > mappingYread[i+1])
      {
        *x=map(p.y, mappingYread[i], mappingYread[i+1], mappingYpos[i], mappingYpos[i+1]);
        break;
      }
    }
    return true;
  }
  return false;
}

void setup(void) {
  Serial.begin(115200);
  Serial.print("Free Heap at start: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" of ");
  Serial.println(ESP.getHeapSize());// Should be 393360
  Serial.println();
  
  // MCUFRIEND_kbv library only
  uint16_t ID = tft.readID();
  Serial.print("TFT ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9486; // write-only shield
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.print("(0,0)");
  tft.setCursor(400, 0);
  tft.print("(400,0)");
  tft.setCursor(400,280);
  tft.print("(400,280)");
  tft.setCursor(0,280);
  tft.print("(0,280)");
  tft.fillCircle(25, 25, 2, GREEN);
  tft.fillCircle(400, 25, 2, BLUE);
  tft.fillCircle(400, 300, 2, RED);
  tft.fillCircle(25, 300, 2, YELLOW);

  Serial.print("Free after: ");
  Serial.println(ESP.getFreeHeap());
  //setupServer();

/*  if (SD.begin())
  {
    tft.println("SD Card initialized.");
    Serial.println("SD Card initialized.");
    hasSD = true;
  }*/
}

void loop(void) 
{
  if(millis()%5000 == 0)
  {
    Serial.print("Free after: ");
    Serial.println(ESP.getFreeHeap());
  }
  server.handleClient();

  // display touched point with colored dot
  uint16_t pixel_x, pixel_y;
  if(touchMapXY(&pixel_x, &pixel_y))
  {
    //tft.fillCircle(pixel_x, pixel_y, 2, brushColor);
    //Serial.print("X: "); Serial.print(pixel_x);
    //Serial.print("Y: "); Serial.println(pixel_y);
  }
  if (Serial.available())
  {
    readPrinter();
  }
  tft.updateDisplay(tft.info);
  delay(1);
}