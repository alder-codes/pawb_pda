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
static int loop_index = 0;


static int margin = 0;
static int panel_width = 0;
static int panel_height = 0;

// at these settings, characters 18 wide and 63 tall 
static int char_w = 28;
static int char_h = 47;

static vector<string> input_buffer = { "" };
static vector<string> input_history = {};
static vector<string> app_menu = { "collect", "inbox", "notes" };
static void (*NextAction)() = NULL;

M5Canvas panel = M5Canvas( &M5.Display );

void setup()
{

  M5.begin();
  Serial.begin(115200);
  M5.Power.begin();
  Wire.begin(25, 32); // M5 CardKB
  while( !SD.begin(4, SPI, 25000000) )
  {
    Serial.println( "SD Card not found." );
    Nope();
  }
  margin = 28;
  panel_width = M5.Display.width()-(margin*2);
  panel_height = M5.Display.height()-(margin*2);
  //Serial.printf( "panel is %d by %d \n", panel_width, panel_height );
  M5.Display.setRotation( 0 );
  M5.Display.setEpdMode( epd_mode_t::epd_fastest );
  panel.createSprite(panel_width,panel_height);
  panel.clear(TFT_WHITE);
  panel.setTextSize(1);
  panel.setTextWrap(true);
  panel.setFont( &fonts::FreeMonoBold18pt7b );
  //panel.loadFont( SD, "/fonts/MonoLisa28.vlw" );
  panel.setTextColor( TFT_BLACK, TFT_WHITE );
  panel.setTextScroll(true);
  panel.setCursor(0,0);
  Serial.printf( "%d by %d\n", panel_width, panel_height );
  panel.pushSprite(margin,margin);
}

void loop()
{
  M5.update();
  try 
  {
    /*
    if( loop_index % 1000000 == 0 )
    {
    }
    */
    
    /*
    if( M5.Touch.getCount() == 1 )
    {
      m5::touch_detail_t t = M5.Touch.getDetail();
      int last_touch_x = app_data.get_int("touch_x");
      int last_touch_y = app_data.get_int("touch_y");
      if( t.x != last_touch_x && t.y != last_touch_y )
      {
        int button_count = app_nav.buttons.size();
        //Serial.printf( "Touch event (%d,%d); Last touch (%d,%d); %d Buttons to check\n", t.x, t.y, app.tx, app.ty, button_count );
        for( int i = 0; i < button_count; i++ )
        {
        }
      }
      app_data.save( "touch_x", t.x );
      app_data.save( "touch_y", t.y );
    }
    */
    // ..........................................................................

    Wire.requestFrom(CARDKB_ADDR, 1);
    while (Wire.available())
    {
      char c = Wire.read();
      if( c > 0  )
      {
        // Handle C  :: input_buffer.back() += c;
        if( c > 0 )
        {
          HandleNewCharacter( c );
        }
      }
    } // while

    // ..........................................................................
    loop_index++;
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in loop: " );
    Serial.println( ex.what() );
  }
}

void Nope()
{
  sleep( 60 * 60 * 60 * 24 );
}

void PanelPrint( string str )
{
  panel.print( str.data() );
  panel.pushSprite(margin,margin);
}

void PanelPrint( char letter )
{
  panel.print( letter );
  panel.pushSprite(margin,margin);
}

void PanelPrintLn( string str )
{
  panel.print( str.data() );
  panel.print( "\n" );
  panel.pushSprite(margin,margin);
}

void PanelPrintLn( char letter )
{
  panel.print( letter );
  panel.print( "\n" );
  panel.pushSprite(margin,margin);
}

// In incriments of char_w and char_h
void MoveCursor( int dx, int dy )
{
  panel.setCursor( panel.getCursorX() + (dx*char_w), panel.getCursorY() + (dy*char_h) );
}

void CursorHome()
{
  // say line/y, but at start
  panel.setCursor( margin, panel.getCursorY() );
}

void HandleNewCharacter( char letter )
{
  try
  {
    Serial.printf( "new char '%c'\n", letter );
    if ( letter == 0x08 ) // backspace
    {
      if( input_buffer.size() > 0 )
      {
        if( input_buffer.back().size() > 0 )
        {
          // remove last character
          input_buffer.back().pop_back();
          MoveCursor(-1,0);
          PanelPrint( " " );
          MoveCursor(-1,0);
        }
        if( input_buffer.back().size() == 0 )
        {
          // that row is empty now
          if( input_buffer.size() > 1 )
          {
            // Drop the drow
            input_buffer.pop_back();
            MoveCursor( 0, -1 );
            int new_x = input_buffer.back().size() * char_w;
            panel.setCursor( new_x, panel.getCursorY() );
          }
        }
      }
    }
    else if ( letter == 0x0D ) // enter key
    {
      // input_buffer.back() += "\n"; technically correct but not good ux
      input_buffer.push_back("");
      PanelPrint( "\n" );
    }
    else if ( letter == 0xA3 ) // fn+enter
    {
      PanelPrint("\nfn+enter\n");
      string joined = pystring::join( "\n", input_buffer );
      Serial.printf( "input item saved: '%s'\n", joined.data() );
      input_history.push_back(joined);
      input_buffer.clear();
      input_buffer.push_back( "" );
    }
    else if  ( letter >= 0x20 ) // characters
    {
      input_buffer.back() += letter;
      PanelPrint( letter );
    }
    else
    {
      string oops =  "unprocessed input = " + letter;
      Serial.println( oops.data() );
    }
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in HandleNewCharacter: " );
    Serial.println( ex.what() );
  }
}

void ReadCommand()
{
  if( input_history.size() > 0 )
  {
    string command = input_history.back();
    Serial.printf( "command given as '%s'\n", command.data() );
    if( command == "menu" )
    {
      for( string option : app_menu )
      {
        panel.println( ( "- " + option ).data() );
      }
      panel.pushSprite(margin,margin);
    }
    else
    {
      PanelPrint( "\nwhat?" );
    }
  }
}
