#include "pawb-pda.h"

using namespace std;

static int panel_width = 0;
static int panel_height = 0;

static int char_w = 0;
static int char_h = 0;
static bool multiline = false;
static bool input_on = false; // use this to denote a time not to iterrupt user
static vector<string> input_buffer = { "" };
static vector<string> input_history = {};

static vector<string> app_menu = { "menu", "collect", "status", "inbox  ", "inbox next", "notes", "battery", "time sync ", "reset" };

static void (*NextAction)() = NULL;

deque<string> inbox_contents = {};
deque<string> notes_contents = {};

deque<int> battery_readings = {};

auto AppFont = &fonts::FreeMonoBold18pt7b;

M5Canvas panel = M5Canvas( &M5.Display );
M5Canvas statusbar = M5Canvas( &M5.Display );
static JsonDocument pawb_config;
static PawbTime pawb_time = PawbTime();

time_t tick;

/*

 ____  _____ _____ _   _ ____
/ ___|| ____|_   _| | | |  _ \
\___ \|  _|   | | | | | | |_) |
 ___) | |___  | | | |_| |  __/
|____/|_____| |_|  \___/|_|


*/
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
  try
  {
    string json = readFile( SD, "/data/config.json" );
    deserializeJson(pawb_config, json);
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in loading config: " );
    Serial.println( ex.what() );
  }
  CheckDirectories();
  IndexFiles();
  panel_width = 100;
  panel_height = 100;
  //Serial.printf( "panel is %d by %d \n", panel_width, panel_height );

  // M5.Display
  M5.Display.setRotation( 0 );
  M5.Display.setEpdMode( epd_mode_t::epd_fastest );

  // Test Sprite to Measure Font Size
  panel.createSprite(panel_width,panel_height);
  panel.setFont( AppFont );
  panel.setCursor(0,0);
  panel.print( "PA" );
  panel.print( "\nW");
  char_w = panel.getCursorX();
  char_h = panel.getCursorY();
  Serial.printf( "char_w(%d) and char_h(%d)\n", char_w, char_h );
  panel.deleteSprite();

  // Ok now make the actual display panel
  panel_width = char_w * (floor( (double) M5.Display.width() / char_w ) - 2);
  panel_height = char_h * ( floor( (double) M5.Display.height() / char_h ) - 2 );
  panel_height -= char_h;
  Serial.printf( "panel_width(%d) and panel_height(%d)\n", panel_width, panel_height );
  panel.createSprite( panel_width, panel_height );
  panel.setFont( AppFont );
  panel.setTextColor( TFT_BLACK, TFT_WHITE );
  panel.clear(TFT_WHITE);
  panel.setTextSize(1);
  panel.setTextWrap(true);
  panel.setTextScroll(true);
  panel.setCursor(0,0);

  // Status Bar
  statusbar.createSprite( M5.Display.width(), char_h*2 );
  statusbar.setFont( &fonts::FreeMonoBold18pt7b );
  statusbar.setTextColor( TFT_WHITE, TFT_BLACK );
  statusbar.clear(TFT_BLACK);
  statusbar.setTextSize(1);
  statusbar.setCursor(0,0);

  // Print the Greeting
  panel.print( "---------\n" );
  panel.print( "|P|A|W|B|\n" );
  panel.print( "---------\n" );
  panel.pushSprite(char_w,char_h);

  pawb_time.GetTime();
  DrawStatusBar();
  Serial.println( "=== loop ===" );
  tick = time(nullptr);

  PrintStatusReport();
}

/*

 _     ___   ___  ____
| |   / _ \ / _ \|  _ \
| |  | | | | | | | |_) |
| |__| |_| | |_| |  __/
|_____\___/ \___/|_|

*/
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
    if( time(nullptr) - tick == 30 )
    {
      pawb_time.GetTime();
      TakeBatteryReading();
      DrawStatusBar();
      tick = time(nullptr);
    }
    
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in loop: " );
    Serial.println( ex.what() );
  }
} // End of loop()

void TakeBatteryReading()
{
  battery_readings.push_front( (int) M5.Power.getBatteryLevel() );
  if( battery_readings.size() > 10 )
  {
    battery_readings.pop_back();
  }
}

double GetBatteryAverage()
{
  int result = -1;
  if( battery_readings.size() > 0 )
  {
    result++;
    for( int reading : battery_readings)
    {
      result += reading;
    }
    result = (double) result / (double) battery_readings.size();
  }
  return result;
}

void Nope()
{
  sleep( 60 * 60 * 60 * 24 );
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

/*
  ____ ___  __  __ __  __    _    _   _ ____  ____
 / ___/ _ \|  \/  |  \/  |  / \  | \ | |  _ \/ ___|
| |  | | | | |\/| | |\/| | / _ \ |  \| | | | \___ \
| |__| |_| | |  | | |  | |/ ___ \| |\  | |_| |___) |
 \____\___/|_|  |_|_|  |_/_/   \_\_| \_|____/|____/

*/
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
      input_on = true;
      multiline = true;
      NextAction = SaveNewItem;
    }
    else if ( command == "status" )
    {
      PrintStatusReport();
    }
    else if ( command == "inbox" )
    {
      if( inbox_contents.size() > 0 )
      {
        for( int i = 0; i < inbox_contents.size(); i++ )
        {
          PanelPrintLn( "- " + inbox_contents[i] );
        }
      }
      else
      {
        PanelPrintLn( "Inbox 0" );
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
        NextAction = InboxKeepOrDelete;
      }
      else
      {
        PanelPrintLn( "Inbox 0" );
      }
    }
    else if ( command == "notes" )
    {
      if( notes_contents.size() > 0 )
      {
        for( int i = 0; i < notes_contents.size(); i++ )
        {
          PanelPrintLn( "- " + notes_contents[i] );
        }
      }
      else
      {
        PanelPrintLn( "No notes" );
      }
    }
    else if ( command == "battery" )
    {
      PanelPrintLn( "Battery: " + to_string(GetBatteryAverage()) + "%" );
    }
    else if ( command == "time sync" )
    {
      if( !pawb_time.IsValid() )
      {
        PanelPrintLn("Clock is out of sync.");
        pawb_time.SyncTime();
      }
    }
    else if ( command == "calendar" )
    {
      // Defaults to this month
      //int year = 
      NextAction = CalendarControls;
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

void PrintStatusReport()
{
  PanelPrintLn("Status Report: ");
  string line = "Inbox (" + to_string( inbox_contents.size() ) + ")";
  PanelPrintLn(line);
  line = "Notes (" + to_string( notes_contents.size() ) + ")";
  PanelPrintLn(line);
  line = "Battery (" + to_string((int)GetBatteryAverage()) + "%)";
  PanelPrintLn(line);
}

void SaveNewItem()
{
  pawb_time.GetTime(); // Update the clock
  string file_name = pawb_time.YYYYMMDD() + "-" + pawb_time.HHMMSS() + ".txt"; // DATE
  string file_path = "/inbox/" + file_name;
  string file_contents = input_history.back();
  if( writeFile( SD, file_path, file_contents ) )
  {
    if( fileExists( SD, file_path ) )
    {
      PanelPrintLn( "New item file saved." );
      IndexFiles();
      NextAction = NULL;
      multiline = false;
      input_on = false;
    }
    else
    {
      PanelPrintLn( "Failed: confirm new file." );
    }
  }
  else
  {
    PanelPrintLn( "Failed: save new file." );
  }
  IndexFiles();
}

void InboxKeepOrDelete()
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
  IndexFiles();
}

void CalendarControls()
{

}


/*

 _____ _ _        ___    _____
|  ___(_) | ___  |_ _|  / / _ \
| |_  | | |/ _ \  | |  / / | | |
|  _| | | |  __/  | | / /| |_| |
|_|   |_|_|\___| |___/_/  \___/

*/

/* ================== File IO ================== */

void IndexFiles()
{
  vector<string> listing = ListDirectory( SD, "/inbox/" );
  inbox_contents = deque(listing.begin(), listing.end());

  listing = ListDirectory( SD, "/notes/" );
  notes_contents = deque(listing.begin(), listing.end());
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


bool fileExists(fs::FS &fs, string path)
{
  return fs.exists( path.data() );
}

bool fileExists(fs::FS &fs, const char *path)
{
  return fs.exists( path );
}

bool deleteFile(fs::FS &fs, const char *path)
{
  if (fs.remove(path))
  {
    PanelPrintLn("Item deleted");
    return true;
  } 
  else
  {
    PanelPrintLn("Delete failed");
    return false;
  }
}

bool renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.print( "Copying " );
  Serial.println( path1 );
  Serial.print( "to " );
  Serial.println( path2 );
  return fs.rename(path1, path2);
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

bool writeFile(fs::FS &fs, string path, string message)
{
  //printf_log("Writing file: %s\n", path);

  File file = fs.open(path.data(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }
  if (file.print(message.data())) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
    return false;
  }
  file.close();
  return true;
}

bool writeFile(fs::FS &fs, const char *path, const char *message)
{
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
    return false;
  }
  file.close();
  return true;
}


/*

 ____  _           _                ____                 _     _
|  _ \(_)___ _ __ | | __ _ _   _   / ___|_ __ __ _ _ __ | |__ (_) ___ ___
| | | | / __| '_ \| |/ _` | | | | | |  _| '__/ _` | '_ \| '_ \| |/ __/ __|
| |_| | \__ \ |_) | | (_| | |_| | | |_| | | | (_| | |_) | | | | | (__\__ \
|____/|_|___/ .__/|_|\__,_|\__, |  \____|_|  \__,_| .__/|_| |_|_|\___|___/
            |_|            |___/                  |_|

*/

/* ================== Graphic Output ================== */

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


void DrawStatusBar()
{
  string humandt = pawb_time.HumanDT(true);
  Serial.print( "Tick... " );
  Serial.println( humandt.data() );
  statusbar.drawCenterString( humandt.data(), M5.Display.width()/2 , char_h * 0.6);
  statusbar.pushSprite( 0, M5.Display.height() - (char_h*2) );
}


/* ================== Peripheral and Sensors ================== */

bool WifiConnect()
{
  try
  {
    time_t start = time(nullptr);  
    std::string wifi_ssid =  pawb_config["Wifi_SSID"].as<std::string>();
    std::string wifi_pass =  pawb_config["Wifi_Pass"].as<std::string>();
    Serial.println( ("Attemptintg to connect to "+wifi_ssid+" with "+wifi_pass).data() );
    WiFi.begin( wifi_ssid.data(), wifi_pass.data() );
    Serial.print( "Wifi " );
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(2000);
      if( time(nullptr) - start > 180000 )
      {
        Serial.print("\n");
        return false;
      }
      Serial.print(".");
    }
    Serial.print(" connected");
    PanelPrintLn("Wifi Connected");
    return true;
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in Wifi Connect: " );
    Serial.println( ex.what() );
  }
}

void WifiOff()
{
  try
  {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    PanelPrintLn("Wifi Off");
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in WifiOff: " );
    Serial.println( ex.what() );
  }
}


/*

 _____ _                  ____  _           _
|_   _(_)_ __ ___   ___  |  _ \(_)___ _ __ | | __ _ _   _
  | | | | '_ ` _ \ / _ \ | | | | / __| '_ \| |/ _` | | | |
  | | | | | | | | |  __/ | |_| | \__ \ |_) | | (_| | |_| |
  |_| |_|_| |_| |_|\___| |____/|_|___/ .__/|_|\__,_|\__, |
                                     |_|            |___/

*/

/* ================== Time Output ================== */


PawbTime::PawbTime()
{
  this->GetTime();
}

void PawbTime::GetTime()
{
  time(&this->now);
  localtime_r(&this->now,&this->tm);
}

string PawbTime::YYYYMMDD()
{
  string output = "";
  output += to_string( this->tm.tm_year + 1900 );
  output += this->u10( this->tm.tm_mon + 1 );
  output += this->u10( this->tm.tm_mday );
  return output;
}

string PawbTime::HHMMSS()
{
  string output = "";
  int offset = pawb_config["UTC"].as<int>();
  int hour12 = (int) this->tm.tm_hour + offset;
  if( this->tm.tm_hour )
  {
    hour12 -= 12;
  }
  output += this->u10( hour12 );
  output += this->u10( this->tm.tm_min );
  output += this->u10( this->tm.tm_sec );
  return output;
}

int PawbTime::GetYear()
{
  return (int) this->tm.tm_year + 1900 ;
}

int PawbTime::GetMonth()
{
  return (int) this->tm.tm_mon + 1;
}

string PawbTime::HumanDT( bool abbreviated )
{
  vector<int> now = {};
  now.push_back( (int) this->tm.tm_year + 1900 ); // 0
  now.push_back( (int) this->tm.tm_mon );         // 1
  now.push_back( (int) this->tm.tm_mday );        // 2
  now.push_back( (int) this->tm.tm_wday );        // 3
  now.push_back( (int) this->tm.tm_hour );        // 4
  now.push_back( (int) this->tm.tm_min );         // 5
  now.push_back( (int) this->tm.tm_sec );         // 6
  string ampm = "AM";

  int offset = pawb_config["UTC"].as<int>();
  now[4] += offset;
  
  if( now[4] > 12 )
  {
    now[4] -= 12;
    ampm = "PM";
  }

  string output = "";
  output += this->DayOfWeek( this->tm.tm_wday, abbreviated) +  " ";
  output += this->MonthName( this->tm.tm_mon, abbreviated );
  output += " " + this->u10(now[2]) + " ";
  output += this->u10(now[4]) + ":";
  output += this->u10(now[5]);
  output += ampm;
  return output;
}

string PawbTime::DayOfWeek( int index, bool abbreviated )
{
  vector<string> names =   {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
  };
  string output = names[index];
  if( abbreviated )
  {
    return output.substr(0,3);
  }
  return output;
}

string PawbTime::MonthName( int index, bool abbreviated )
{
  vector<string> names =   {
    "January", "February", "March", "April", "May", "June", 
    "July", "August", "September", "October", "November", "December"
  };
  string output = names[index];
  if( abbreviated )
  {
    return output.substr(0,3);
  }
  return output;
}

string PawbTime::u10( int number )
{
  string output = to_string(number);
  if( number < 10 )
  {
    output = "0" + output;
  }
  return output;
}

bool PawbTime::IsValid()
{
  bool valid_time = true;
  if( (this->tm.tm_year+1900) < 2025 )
  {
    Serial.println( "Year less than 2025" );
    return false;
  }
  if( this->tm.tm_mon < 0 || this->tm.tm_mon > 11 )
  {
    Serial.printf( "Month out of range: %d \n", (int) this->tm.tm_mon );
    return false;
  }
  return valid_time;
}


//-----------------------------------------------------------------------------
void PawbTime::SyncTime()
{
  try
  {
    Serial.println( "Syncing time. Please wait." );
    bool op_success = false;
    const std::vector<std::string> servers = { "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org" };
    if( WifiConnect() )
    {
      const std::string timezone = pawb_config["TZ"].as<std::string>();
      configTzTime( timezone.data(), servers[0].data(), servers[1].data(), servers[2].data());
      if( this->SntpConnect() )
      {
        if( this->SyncRtc() )
        {
          op_success = true;
        }
        else 
        {
          Serial.println( "SyncRTC failed." );
        }
      }
      else
      {
        Serial.println( "SNTP Connect Failed" );
      }
    }
    else
    {
      Serial.println( "Wifi connect failed." );
    }
    if( op_success )
    {
      Serial.println( "Sync time successful." );
    }
    else
    {
      Serial.println( "Sync time failed." );
    }
    WifiOff();
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in Wifi Connect: " );
    Serial.println( ex.what() );
  }
}


bool PawbTime::SntpConnect()
{
  try
  {
    bool success = true;
    time_t start = time(nullptr);  
    #if SNTP_ENABLED
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
    {
      delay(500);
    }
    #else
    delay(1600);
    struct tm timeInfo;
    while (!getLocalTime(&timeInfo, 1000))
    {
      delay(500);
    };
    #endif
    Serial.printf( "\nSNTP Time: \n", time(nullptr)-start );
    return true;
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in SNTP Connected: " );
    Serial.println( ex.what() );
  }
}



bool PawbTime::SyncRtc()
{
  try
  {
    time_t start = time(nullptr);  
    time_t t = time(nullptr) + 1;  // Advance one second.
    while (t > time(nullptr))
        ;  /// Synchronization in seconds
    M5.Rtc.setDateTime(gmtime(&t));
    bool confirmed = false;
    while( !confirmed )
    {
      delay(500);
      if( time(nullptr) - start > 180000 )
      {
        Serial.println( "SyncRTC timedout.");
        return false;
      }
      this->GetTime(); // update
      if( this->IsValid() ) // is it valid yet?
      {
        Serial.println( "Update: time confirmed valid.");
        confirmed = true;
        return true;
      }
    }
    return true;
  }
  catch(const std::exception & ex)
  {
    Serial.println( "\nexception in SyncRTC: " );
    Serial.println( ex.what() );
  }
}





