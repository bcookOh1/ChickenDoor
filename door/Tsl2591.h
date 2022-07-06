
/// file: Si7021.h header for Si7021 using a the I2C base class  
/// author: Bennett Cook
/// date: 01-31-2021
/// description: 
/// ref: https://github.com/adafruit/Adafruit_TSL2591_Library/blob/master/Adafruit_TSL2591.h (and .cpp)
/// this code follows closely with the c++ code from the adafruit github above 
/// ref: tsl2591_datasheet_1633.pdf

// header guard
#ifndef TSL2591_H
#define TSL2591_H

#include "I2C.h"

#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <map>


using namespace std;

// TSL2591 constants  
const unsigned char TSL2591_I2C_ADDRESS = 0x29;
const unsigned char TSL2591_COMMAND_BITS = 0xA0;
const unsigned char TSL2591_REGISTER_ENABLE = 0x00; 
const unsigned char TSL2591_REGISTER_CONTROL = 0x01;
const unsigned char TSL2591_REGISTER_ID = 0x12;
const unsigned char TSL2591_REGISTER_STATUS = 0x13;
const unsigned char TSL2591_REGISTER_CH0_LO = 0x14;
const unsigned char TSL2591_REGISTER_CH0_HI = 0x15;
const unsigned char TSL2591_REGISTER_CH1_LO = 0x16;
const unsigned char TSL2591_REGISTER_CH1_HI = 0x17;

const unsigned char TSL2591_POWEROFF = 0x00; 
const unsigned char TSL2591_POWERON = 0x01;  
const unsigned char TSL2591_ENABLE_AEN = 0x02;

// note: the gain and the read time are written together,
// the gain is the upper 4 bits and the time is the lower 4 bits.
// to use just add together and write 
 
// gain values
const unsigned char TSL2591_GAIN_LOW = 0x00;  // 1x
const unsigned char TSL2591_GAIN_MID = 0x10;  // 25x
const unsigned char TSL2591_GAIN_HIGH = 0x20; // 428x
const unsigned char TSL2591_GAIN_MAX = 0x30;  // 9876x

// read (integration) time
const unsigned char TSL2591_READ_TIME_100MS = 0x00;  // 100 ms
const unsigned char TSL2591_READ_TIME_200MS = 0x01;  // 200 ms
const unsigned char TSL2591_READ_TIME_300MS = 0x02;  // 300 ms
const unsigned char TSL2591_READ_TIME_400MS = 0x03;  // 400 ms
const unsigned char TSL2591_READ_TIME_500MS = 0x04;  // 500 ms
const unsigned char TSL2591_READ_TIME_600MS = 0x05;  // 600 ms

// interrupt not used in this app  
const unsigned char TSL2591_CLEAR_INT = 0xE7;

// this is the actual devive id that should be returned 
// when querying the device, use as a check in open()  
const unsigned char TSL2591_DEVICE_ID_VALUE = 0x50;

// lux conversion constants
const float LUX_DF = 408.0f;
const float LUX_COEFFICIENT_1 = 1.64f;  
const float LUX_COEFFICIENT_2 = 0.59f;  
const float LUX_COEFFICIENT_3 = 0.86f;


const float MAX_LUX_VALUE = 88000.0f; // from adafruit 
const float MIN_LUX_VALUE = 0.0f;

// the least significant bit = 1 is the important part 1 = valid read
// mask the other bits 
const unsigned char TSL2591_VALID_READ_STATUS = 0x01; 

// map for lux calc
const map<unsigned char, float> GAIN_VALUES = {
   {TSL2591_GAIN_LOW, 1.0f},
   {TSL2591_GAIN_MID, 25.0f},
   {TSL2591_GAIN_HIGH, 428.0f},
   {TSL2591_GAIN_MAX, 9876.0f},
}; // end gain map definition


// map for lux calc
const map<unsigned char, float> READ_TIME_VALUES = {
   {TSL2591_READ_TIME_100MS, 100.0f},
   {TSL2591_READ_TIME_200MS, 200.0f},
   {TSL2591_READ_TIME_300MS, 300.0f},
   {TSL2591_READ_TIME_400MS, 400.0f},
   {TSL2591_READ_TIME_500MS, 500.0f},
   {TSL2591_READ_TIME_600MS, 600.0f},
}; // end read time (integration) map definition


class Tsl2591 : public I2C {
public:
   Tsl2591();
   ~Tsl2591();

   int ReadSensor();

   float GetLightLevel() {return _lightLevel;}
   unsigned GetRawLightLevel() {return _rawLightLevel;}
   unsigned char GetDeviceStatus() {return _status;}
   unsigned char GetDeviceId() {return _id;}
   string LightLevelToString();

private:

   int PowerOn(bool on);
   int ReadDeviceId();
   int ReadDeviceStatus();
   int SetIntegrationAndGain(unsigned char gain, unsigned char integration);
   int ReadLightLevels();
   int ReadRegister(unsigned char regNum, unsigned char &val);
   float CalculateLux(unsigned ch0, unsigned ch1);

   unsigned _rawLightLevel;
   float _lightLevel;
   float _gain;
   float _integration;
   unsigned char _status;
   unsigned char _id;

}; // end class

#endif  // end header guard
