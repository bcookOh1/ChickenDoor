
/// file: Si7021.h header for Si7021 using a the I2C base class  
/// author: Bennett Cook
/// date: 01-31-2021
/// description: 
/// ref: https://www.silabs.com/documents/public/data-sheets/Si7021-A20.pdf


// header guard
#ifndef SI7021_H
#define SI7021_H

#include "I2C.h"

#include <string>
#include <sstream>
#include <thread>
#include <chrono>

using namespace std;

// SI7021 I2C device address 
const unsigned char SI7021_I2C_ADDRESS = 0x40;

// commands to read immediately (no hold master) 
const unsigned char CMD_READ_HUMIDITY_NO_HOLD = 0xF5;
const unsigned char CMD_READ_TEMP_NO_HOLD = 0xF3;

const int WAIT_READ_MS = 1000;

enum class SI7021_READINGS : int {
   Both = 0,
   TemperatureOnly,
   HumidityOnly
};  // end enum


class Si7021 : public I2C {
public:
   Si7021();
   ~Si7021();

   int operator()(enum SI7021_READINGS reading);
   int ReadSensor(enum SI7021_READINGS reading);

   float GetHumidityReading() {return _humidity;}
   string GetHumidityUnits() {return "%";}
   float GetTempReading(bool tempInC = true) {return (tempInC ? _temperature : _temperature * 1.8 + 32);}
   string GetTemperatureUnits(bool tempInC = true){return (tempInC ? "degC" : "degF");}

   string HumidityToString();
   string TempToString(bool tempInC = true);

private:

   float _humidity;
   float _temperature;

   // read individuals  
   int ReadHumidity();
   int ReadTemp();

}; // end class

#endif  // end header guard
