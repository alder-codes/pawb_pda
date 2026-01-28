#ifndef PAWB_PDA_H
#define PAWB_PDA_H

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <deque>
#include <vector>
#include <string>
#include <unordered_map>
#include <WiFi.h>
#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>
#include <Wire.h>
#include <pystring.h>
#include <M5GFX.h>
#include <M5Unified.h>

// Different versions of the framework have different SNTP header file names and availability.
#if __has_include(<esp_sntp.h>)
#include <esp_sntp.h>
#define SNTP_ENABLED 1
#elif __has_include(<sntp.h>)
#include <sntp.h>
#define SNTP_ENABLED 1
#endif

#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif


using namespace std;


static const int CARDKB_ADDR = 0x5F;


void setup();
void loop();
void Nope();
void PanelPrint( string str );
void PanelPrint( char letter );
void PanelPrintLn( string str );
void PanelPrintLn( char letter );
void MoveCursor( int dx, int dy );
void CursorHome();
void HandleNewCharacter( char letter );
void ReadCommand();
void SaveNewItem();
vector<string> ListDirectory(fs::FS &fs, const char *dirname);


#endif