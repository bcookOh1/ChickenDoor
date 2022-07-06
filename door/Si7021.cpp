
// BCook 1/31/2021, adapted from control everything company
// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// SI7021
// This code is designed to work with the SI7021_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Humidity?sku=SI7021_I2CS#tabs-0-product_tabset-2


#include "Si7021.h"

Si7021::Si7021() : I2C(SI7021_I2C_ADDRESS) {
   _temperature = 0.0f;
   _humidity = 0.0f;
} // end ctor 


Si7021::~Si7021(){
} // end dtor 


int Si7021::ReadHumidity() {
   int ret = 0;

	// send humidity measurement command
	unsigned char config[1] = {CMD_READ_HUMIDITY_NO_HOLD};
	write(_fd, config, 1);
   this_thread::sleep_for(chrono::milliseconds(WAIT_READ_MS));

	// read 2 bytes of humidity data, humidity msb, humidity lsb
	char data[2] = {0};
	if(read(_fd, data, 2) != 2) {
      ret = -1;
      _error = "humidity read error";
	}
	else {
		// convert the data
      int combined = ((static_cast<int>(data[0]) << 8) + data[1]);
		_humidity = ((combined * 125.0) / 65536.0) - 6;
	} // end if

   return ret;
} // end readHumidity

int Si7021::ReadTemp(){
   int ret = 0;

  	// send temperature measurement command
   unsigned char config[1] = {CMD_READ_TEMP_NO_HOLD};
	write(_fd, config, 1); 
   this_thread::sleep_for(chrono::milliseconds(WAIT_READ_MS));

	// read 2 bytes of temperature data, data[0]=msb, data[1]=lsb
   char data[2] = {0};
	if(read(_fd, data, 2) != 2) {
      ret = -1;
		_error = "temperature read error";
	}
	else {
      int combined = ((static_cast<int>(data[0]) << 8) + data[1]);
		_temperature = ((combined * 175.72) / 65536.0) - 46.85;
	} // end if 
 
   return ret;
} // end ReadTemp


string Si7021::HumidityToString(){
   string ret;

   ostringstream oss;
   oss << fixed << setprecision(1) << _humidity;
   ret = oss.str();

   return ret;
} // end HumidityToString


string Si7021::TempToString(bool tempInC){
   string ret;

   ostringstream oss;
   oss << fixed << setprecision(1) << GetTempReading(tempInC);
   ret = oss.str();
   
   return ret;
} // end TempToString



int Si7021::operator()(enum SI7021_READINGS reading){
   return ReadSensor(reading);
} // end operator()


int Si7021::ReadSensor(enum SI7021_READINGS reading){
   int ret = 0;

   _temperature = 0.0f;
   _humidity = 0.0f;
   _error = "";

   int result = Open();
   if(result == 0) {

      switch(reading){
      case SI7021_READINGS::Both: 
         result = ReadHumidity();
         if(result == 0) 
            result = ReadTemp();

         break;
      case SI7021_READINGS::HumidityOnly: 
         result = ReadHumidity();

         break;
      case SI7021_READINGS::TemperatureOnly: 
         result = ReadTemp();
         
         break;
      } // end switch

      result = Close();
   } // end if

   if(result != 0) {
      ret = -1;
   } // end if 

   return ret;
} // end ReadSensor

