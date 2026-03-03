//
// Created by Alice Rhodes on 3/3/26.
//

#ifndef UNTITLED_HARDWARE_ESP32_H
#define UNTITLED_HARDWARE_ESP32_H

#include <Arduino.h>

class PimHardware
{
  public:
    static void init();
    static void log( const String& message );
};


#endif //UNTITLED_HARDWARE_ESP32_H