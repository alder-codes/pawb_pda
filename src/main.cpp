//#define PLATFORM_LINUX
#define PLATFORM_ESP32

#ifdef PLATFORM_ESP32
#include <Arduino.h>
#include "Hardware_ESP32.h"
#endif

#ifdef PLATFORM_LINUX
#include "../linux/Hardware_Linux.h"
#endif

static int is_running = 1;

void setup()
{
  PimHardware::init();
  // write your initialization code here
}

void loop()
{
  // write your code here
}


#ifdef PLATFORM_LINUX
int main()
{
  setup();
  while( is_running > 0 )
  {
    loop();
  }
  return 0;
}
#endif