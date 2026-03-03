#ifndef PAWB_HARDWARE_LINUX_H
#define PAWB_HARDWARE_LINUX_H

#include <string>

using namespace std;

class PimHardware
{
  public:
    static void init();
    static void log( const string& message );
};


#endif //PAWB_HARDWARE_LINUX_H