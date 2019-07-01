
// Contains all web interface functions

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <SD.h>

// WiFi Server
extern WebServer server;
extern const char* ssid;
extern const char* password;
extern const char* host;
extern const char* webFolder;

extern bool hasSD;

void returnOK();
void returnFail(String msg);
bool loadFromSdCard(String path);
void handleFileUpload();
void deleteRecursive(String path);
void handleDelete();
void handleCreate();
void printDirectory();
void handleNotFound();
void setupServer();
bool connectWiFi(const char* ssid_l, const char* pass);



//static bool hasSD;
#endif
