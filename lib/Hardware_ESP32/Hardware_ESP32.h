#ifndef PAWB_HARDWARE_ESP32_H
#define PAWB_HARDWARE_ESP32_H

#include <Arduino.h>
#include <string>

using namespace std;


class PimHardware
{
  public:
    static void init();
    static void loop();
    static void log( const string& message );
    static void handle_exception( const exception &e );
};


#endif //PAWB_HARDWARE_ESP32_H