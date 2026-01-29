#include "pawb-pda.h"

using namespace std;

static int loop_index = 0;

static int panel_width = 0;
static int panel_height = 0;

static int char_w = 0;
static int char_h = 0;
static bool multiline = false;
static vector<string> input_buffer = { "" };
static vector<string> input_history = {};
static vector<string> app_menu = { "collect", "inbox", "notes", "reset" };
static void (*NextAction)() = NULL;

deque<string> inbox_contents = {};

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
  Serial.println( "=== start ===" );
  CheckDirectories();
  vector<string> listing = ListDirectory( SD, "/inbox/" );
  inbox_contents = deque(listing.begin(), listing.end());
  panel_width = 100;
  panel_height = 100;
  //Serial.printf( "panel is %d by %d \n", panel_width, panel_height );
  M5.Display.setRotation( 0 );
  M5.Display.setEpdMode( epd_mode_t::epd_fastest );
  panel.createSprite(panel_width,panel_height);
  panel.setFont( &fonts::FreeMonoBold18pt7b );
  panel.setCursor(0,0);
  panel.print( "PA" );
  panel.print( "\nW");
  char_w = panel.getCursorX();
  char_h = panel.getCursorY();
  Serial.printf( "char_w(%d) and char_h(%d)\n", char_w, char_h );
  panel.print( "B\n");
  panel.deleteSprite();
  panel_width = char_w * (floor( (double) M5.Display.width() / char_w ) - 2);
  panel_height = char_h * ( floor( (double) M5.Display.height() / char_h ) - 2 );
  Serial.printf( "panel_width(%d) and panel_height(%d)\n", panel_width, panel_height );
  panel.createSprite( panel_width, panel_height );
  panel.setFont( &fonts::FreeMonoBold18pt7b );
  panel.setTextColor( TFT_BLACK, TFT_WHITE );
  panel.clear(TFT_WHITE);
  panel.setTextSize(1);
  panel.setTextWrap(true);
  panel.setTextScroll(true);
  panel.setCursor(0,0);
  panel.print( "PA" );
  panel.print( "\nWB\n" );
  panel.pushSprite(char_w,char_h);
  Serial.println( "=== loop ===" );
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

void CheckDirectories()
{
  // Check if the key directories exist on the SD card and if they don't then create them
  // inbox
  // notes
  vector<string> key_directories = { "inbox", "notes" };
  for( int i = 0; i < key_directories.size(); i++ )
  {
    string dir = "/"+key_directories[i];
    File root = SD.open( dir.data() );
    if( !root || !root.isDirectory() )
    {
      // Directory doesn't exist so create it
      if( SD.mkdir( dir.data() ) )
      {
        Serial.println( ("Created: " + dir).data() );
      }
      else
      {
        Serial.println( ("Failed to create: " + dir).data() );
      }
    }
  }

}

void Nope()
{
  sleep( 60 * 60 * 60 * 24 );
}

void PanelPrint( string str )
{
  panel.print( str.data() );
  panel.pushSprite(char_w,char_h);
}

void PanelPrint( char letter )
{
  panel.print( letter );
  panel.pushSprite(char_w,char_h);
}

void PanelPrintLn( string str )
{
  panel.print( str.data() );
  panel.print( "\n" );
  panel.pushSprite(char_w,char_h);
}

void PanelPrintLn( char letter )
{
  panel.print( letter );
  panel.print( "\n" );
  panel.pushSprite(char_w,char_h);
}

// In incriments of char_w and char_h
void MoveCursor( int dx, int dy )
{
  panel.setCursor( panel.getCursorX() + (dx*char_w), panel.getCursorY() + (dy*char_h) );
}

void CursorHome()
{
  // say line/y, but at start
  panel.setCursor( char_w, panel.getCursorY() );
}

void HandleNewCharacter( char letter )
{
  try
  {
    if ( letter == 0x08 ) // backspace
    {
      Serial.println( "backspace" );
      if( input_buffer.size() > 0 )
      {
        if( input_buffer.back().size() > 0 )
        {
          // remove last character
          input_buffer.back().pop_back();
          MoveCursor(-1,0);
          PanelPrint( " " );
          MoveCursor(-1,0);
          Serial.println( "Character removed" );
        }
        if( input_buffer.back().size() == 0 )
        {
          Serial.println( "input buffer line is now empty" );
          // that row is empty now
          if( input_buffer.size() > 1 )
          {
            // Drop the drow
            input_buffer.pop_back();
            Serial.println( "removing empty line" );
            MoveCursor( 0, -1 );
            int new_x = input_buffer.back().size() * char_w;
            panel.setCursor( new_x, panel.getCursorY() );
          }
          else
          {
            Serial.println( "input buffer is now empty" );
          }
        }
      }
      else
      {
        Serial.println( "input buffer already empty" );
      }
    }
    else if ( letter == 0x0D ) // enter key: only adds return multi-line
    {
      if( multiline )
      {
        // input_buffer.back() += "\n"; technically correct but not good ux
        input_buffer.push_back("");
        Serial.println( "Added New Line" );
        PanelPrint( "\n" );
      }
      else
      {
        Serial.println( "New char is 'return' but multi-line is off" );
      }
    }
    else if ( letter == 0xA3 ) // fn+enter: is always the submit input command
    {
      Serial.println("fn+enter");
      string joined = pystring::join( "\n", input_buffer );
      Serial.printf( "input item saved: '%s'\n", joined.data() );
      input_history.push_back(joined);
      input_buffer.clear();
      input_buffer.push_back( "" );
      PanelPrintLn(" <- ");
      if( NextAction == NULL )
      {
        ReadCommand();
      }
      else
      {
        NextAction();
      }
    }
    else if  ( letter >= 0x20 ) // characters
    {
      Serial.printf( "new char '%c'\n", letter );
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
      panel.pushSprite(char_w,char_h);
    }
    else if ( command == "collect" )
    {
      PanelPrintLn("input: ");
      multiline = true;
      NextAction = SaveNewItem;
    }
    else if ( command == "inbox" )
    {
      for( int i = 0; i < inbox_contents.size(); i++ )
      {
        PanelPrintLn( "- " + inbox_contents[i] );
      }
    }
    else if ( command == "inbox next" )
    {
      if( inbox_contents.size() > 0 )
      {
        string path = "/inbox/" + inbox_contents.front();
        string item = readFile( SD, path.data() );
        PanelPrintLn( "opening: " + inbox_contents.front() );
        PanelPrintLn( item );
        PanelPrintLn( "(k)eep or (d)elete?" );
        multiline = false;
        NextAction = KeepOrDelete;
      }
      else
      {
        PanelPrintLn( "Inbox 0" );
      }
    }
    else if ( command == "reset" )
    {
      PanelPrintLn("Restarting!");
      ESP.restart();
    }
    else
    {
      PanelPrintLn( "what?" );
    }
  }
}


void SaveNewItem()
{
  PanelPrintLn("[Save To Inbox]");
  NextAction = NULL;
  multiline = false;
}

void KeepOrDelete()
{
  string cmd = input_history.back();
  if( cmd == "k" )
  {
    PanelPrintLn( "Moving item to notes." );
    string file_name = inbox_contents.front();
    string oldPath = "/inbox/" + file_name;
    string newPath = "/notes/" + file_name;
    if( renameFile( SD, oldPath.data(), newPath.data() ) )
    {
      inbox_contents.pop_front();
      NextAction = NULL;
      multiline = false;
    }
  }
  else if ( cmd == "d" )
  {
    PanelPrintLn( "Deleting item." );
    if( deleteFile( SD, ("/inbox/" + inbox_contents.front()).data() ) )
    {
      inbox_contents.pop_front();
      NextAction = NULL;
      multiline = false;
    }
  }
  else
  {
    PanelPrintLn( "what?" );
  }
}

bool deleteFile(fs::FS &fs, const char *path)
{
  if (fs.remove(path)) {
    PanelPrintLn("Item deleted");
    return true;
  } else {
    PanelPrintLn("Delete failed");
    return false;
  }
}

bool renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  if (fs.rename(path1, path2)) {
    PanelPrintLn("Item moved");
    return true;
  } else {
    PanelPrintLn("Move failed");
    return false;
  }
}

vector<string> ListDirectory(fs::FS &fs, const char *dirname)
{
  vector<string> listing = {};

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return listing;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return listing;
  }

  File file = root.openNextFile();
  while (file)
  {
    if( !file.isDirectory() )
    {
      auto file_name = file.name();
      listing.push_back( file_name );
    }
    file = root.openNextFile();
  }
  return listing;
}

string readFile(fs::FS &fs, const char *path)
{
  string buffer;
  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return "";
  }
  // this is one character at a time and it is
  // coming in as the unicode number
  while (file.available()) {
    buffer += file.read();
  }
  file.close();
  return buffer;
}


