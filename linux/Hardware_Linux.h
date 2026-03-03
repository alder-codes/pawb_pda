#ifndef PIM_HARDWARE_LINUX_H
#define PIM_HARDWARE_LINUX_H

#include <string>

using namespace std;

class PimHardware
{
  public:
    static void init();
    static void log( const string& message );
};


#endif //PIM_HARDWARE_LINUX_H