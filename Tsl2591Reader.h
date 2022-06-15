
// header guard
#ifndef TSl2591READER_H
#define TSl2591READER_H

#include <iostream>
#include <iomanip>
#include <string>

#include "Reader.h"
#include "Tsl2591.h"

using namespace std;

// this class must set both 
// _status = ReaderStatus::Complete;
// _status = ReaderStatus::Error;


// data from the sensor 
struct Tsl2591Data {
   float lightLevel;
   string lightLevelStr;
   unsigned short rawlightLevel;
}; // end struct

class Tsl2591Reader : public Reader {
public:

   Tsl2591Reader();
   virtual ~Tsl2591Reader();
   int RunTask() override;
   Tsl2591Data GetData();

private:

   Tsl2591 _sensor;
   Tsl2591Data _sensorData;

}; // end class 




#endif // end header guard

