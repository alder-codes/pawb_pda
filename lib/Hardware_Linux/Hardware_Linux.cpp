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

void PimHardware::handle_exception( const exception &e )
{
  cout << "-- exception --" << endl;
  cout << e.what() << endl;
}

void PimHardware::loop()
{
  try
  {
    PimHardware::log("-- loop --");
  }
  catch (const exception &e)
  {
    PimHardware::handle_exception( e );
  }
}