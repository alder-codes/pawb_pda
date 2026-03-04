// Only Uncomment one platform at a time
//#define PLATFORM_LINUX
#define PLATFORM_ESP32
//----- end of Platform selection -----

// The make files handles path differently than platform.io
// Paths should be relative
#ifdef PLATFORM_LINUX
#include "../lib/Hardware_Linux/Hardware_Linux.h"
#include "../lib/PimApp/PimApp.h"
#endif

// Platform.io manages paths so libraries can be added
// without relative location
#ifdef PLATFORM_ESP32
#include <Hardware_ESP32.h>
#include <PimApp.h>
#endif



static int is_running = 1;

void setup()
{
  PimHardware::init();
}

void loop()
{
  PimHardware::loop();
}


#ifdef PLATFORM_LINUX
int main()
{
  setup();
  while( is_running > 0 )
  {
    try
    {
      loop();
    }
    catch(std::exception& e)
    {
      PimHardware::handle_exception(e);
      is_running = 0;
    }
  }
  return 0;
}
#endif