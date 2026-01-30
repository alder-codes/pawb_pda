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




void HandleNewCharacter( char letter );
void ReadCommand();
void SaveNewItem();
void KeepOrDelete();


void PanelPrint( string str );
void PanelPrint( char letter );
void PanelPrintLn( string str );
void PanelPrintLn( char letter );
void MoveCursor( int dx, int dy );
void CursorHome();


void IndexFiles();
void CheckDirectories();
bool deleteFile(fs::FS &fs, const char *path);
bool renameFile(fs::FS &fs, const char *path1, const char *path2);
vector<string> ListDirectory(fs::FS &fs, const char *dirname);
string readFile(fs::FS &fs, const char *path);








class PawbTime
{
  public:
    PawbTime();
    void GetTime();
    string YYYYMMDD();
    string HHMMSS();
    string HumanDT( bool abbreviated );
    bool IsValid();
    void SyncTime();
  private:
    string DayOfWeek( int index, bool abbreviated );
    string MonthName( int index, bool abbreviated );
    string u10( int number );
    bool SntpConnect();
    bool SyncRtc();

    time_t now;
    tm tm;
};





#endif