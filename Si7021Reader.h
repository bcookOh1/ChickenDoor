
// header guard
#ifndef SI7021READER_H
#define SI7021READER_H

#include <iostream>
#include <iomanip>
#include <string>

#include "Reader.h"
#include "Si7021.h"
// #include "Util.h"

using namespace std;

// this class must set both 
// _status = ReaderStatus::Complete;
// _status = ReaderStatus::Error;


// data from the sensor 
struct Si7021Data {
   string humidity;
   string humidityUnits;
   string temperature;
   string TemperatureUnits;
}; // end struct

class Si7021Reader : public Reader {
public:

   Si7021Reader();
   virtual ~Si7021Reader();
   int RunTask() override;
   Si7021Data GetData();

private:

   Si7021 _sensor;
   Si7021Data _sensorData;

}; // end class 




#endif // end header guard

