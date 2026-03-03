#include "Hardware_Linux.h"

#include <iostream>

using namespace std;


void PimHardware::init()
{
  cout << "init" << endl;
}

void PimHardware::log( const string& message )
{
  cout << message << endl;
}