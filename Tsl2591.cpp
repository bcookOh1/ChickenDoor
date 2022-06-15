
// BCook 5/2/2022, adapted from Adafruit Tsl2591 c++ code on github 

#include "Tsl2591.h"
#include <cassert>


Tsl2591::Tsl2591() : I2C(TSL2591_I2C_ADDRESS) {
   _lightLevel = 0;
   _gain = static_cast<float>(TSL2591_GAIN_MID);
   _integration = static_cast<float>(TSL2591_READ_TIME_300MS);
} // end ctor 


Tsl2591::~Tsl2591(){
} // end dtor 

int Tsl2591::PowerOn(bool on){
   int ret = 0;

   unsigned char state = (on == true ? (TSL2591_POWERON | TSL2591_ENABLE_AEN) : TSL2591_POWEROFF);
   unsigned char command[2] = {TSL2591_COMMAND_BITS | TSL2591_REGISTER_ENABLE, state}; 
   // cout << "PowerOn: " << static_cast<unsigned short>(command[0]) << " " << static_cast<unsigned short>(command[1]) << endl;
   if(write(_fd, command, 2) != 2){
      ret = -1;
      _error = "error on setting power state";
   } // end if 

   return ret;
} // end PowerOn


int Tsl2591::ReadDeviceId(){
   int ret = 0;
   _id = 0;

   unsigned char command = TSL2591_COMMAND_BITS | TSL2591_REGISTER_ID;  
   if(write(_fd, &command, 1) == 1){
            
      unsigned char data[1] = {0};
      if(read(_fd, data, 1) == 1){
         _id = data[0];
      }
      else {
         ret = -1;
         _error = "read id error";
	   } // end if
      
   }
   else {
      ret = -1;
      _error = "error on request id";
   } // end if 

   return ret;
} // end ReadDeviceId


int Tsl2591::ReadDeviceStatus(){
   int ret = 0;
   _status = 0;

   unsigned char command = TSL2591_COMMAND_BITS | TSL2591_REGISTER_STATUS;  
   if(write(_fd, &command, 1) == 1){
            
      unsigned char data[1] = {0};
      if(read(_fd, data, 1) == 1){
         _status = data[0];
      }
      else {
         ret = -1;
         _error = "read status error";
	   } // end if
      
   }
   else {
      ret = -1;
      _error = "error on request status";
   } // end if 

   return ret;
} // end ReadDeviceStatus


int Tsl2591::SetIntegrationAndGain(unsigned char gain, unsigned char integration){
   int ret = 0;
   
   _gain = static_cast<float>(gain);
   _integration = static_cast<float>(integration);

   unsigned char command[2]; 
   command[0] = TSL2591_COMMAND_BITS | TSL2591_REGISTER_CONTROL; 
   command[1] = gain | integration; 

   if(write(_fd, command, 2) != 2){
      ret = -1;
      _error = "error on set integration and gain";
   } // end if 
   
   return ret;
} // end SetIntegrationAndGain


int Tsl2591::ReadLightLevels() {
   int ret = 0;
   int result = 0;
   
   unsigned char v1 = 0;
   unsigned char v2 = 0;

   result = ReadRegister(TSL2591_REGISTER_CH0_LO, v1);
   assert(result == 0);

   result = ReadRegister(TSL2591_REGISTER_CH0_HI, v2);
   assert(result == 0);

   unsigned short ch0 = (static_cast<unsigned short>(v2) << 8) | static_cast<unsigned short>(v1);

   result = ReadRegister(TSL2591_REGISTER_CH1_LO, v1);
   assert(result == 0);

   result = ReadRegister(TSL2591_REGISTER_CH1_HI, v2);
   assert(result == 0);

   unsigned short ch1 = (static_cast<unsigned short>(v2) << 8) | static_cast<unsigned short>(v1);

   // combine channels into a raw light value
   _rawLightLevel = (static_cast<unsigned>(ch1) << 16) | ch0; 

   // calc the lux value from the two different sensors
   _lightLevel = CalculateLux(ch0, ch1);

   return ret;
} // end ReadLightLevels


float Tsl2591::CalculateLux(unsigned ch0, unsigned ch1){
   float ret = 0;

   // check for saturation
   if(ch0 == 0xffff || ch1 == 0xffff){
      return MAX_LUX_VALUE;
   }  // end if 

   // check for zero
   if(ch0 == 0x0 || ch1 == 0x0){
      return MIN_LUX_VALUE;
   }  // end if 

   // precondition channel data so na is not calculated
   // if cho == ch1 will produce 0.0/cpl and an na  
   if(ch0 == ch1)
      ch1--; // reducing by 1 has little or no affect

   // alternate lux calc from adafruit 
   float cpl = (_integration * _gain) / LUX_DF;
   ret = (static_cast<float>(ch0) - static_cast<float>(ch1)) * 
         (1.0f - (static_cast<float>(ch1) / static_cast<float>(ch0))) / cpl;

   return ret;
} // end CalculateLux


// read an 8bit register 
int Tsl2591::ReadRegister(unsigned char regNum, unsigned char &val){
   int ret = 0;

   unsigned char command[1];
   command[0] = TSL2591_COMMAND_BITS | regNum; 
   // cout << "ReadRegister: " << static_cast<unsigned short>(command[0]) << endl;
   if(write(_fd, &command, 1) == 1){
            
      unsigned char data[1] = {0};
      if(read(_fd, data, 1) == 1){
         val = data[0];
         // cout << "data: " << static_cast<unsigned short>(data[0]) << endl;
      }
      else {
         ret = -1;
         _error = "read data error";
	   } // end if
      
   }
   else {
      ret = -1;
      _error = "error on read data request";
   } // end if 

   return ret;
} // end ReadRegister


string Tsl2591::LightLevelToString(){
   string ret;

   ostringstream oss;
   oss << fixed << setprecision(1) << _lightLevel;
   ret = oss.str();

   return ret;
} // end LightLevelToString


int Tsl2591::ReadSensor(){
   int ret = 0;

   _lightLevel = 0;
   _error = "";

   int result = Open();
   if(result == 0) {

      result = PowerOn(true);
      if(result != 0) {
         Close();
         return -1;
      } // end if 

      result = SetIntegrationAndGain(TSL2591_GAIN_MID, TSL2591_READ_TIME_300MS);
      if(result != 0) {
         PowerOn(false);
         Close();
         return -1;
      } // end if 

      result = PowerOn(false);
      if(result != 0) {
         Close();
         return -1;
      } // end if 
 
      this_thread::sleep_for(chrono::milliseconds(100ms));

      result = PowerOn(true);
      if(result != 0) {
         Close();
         return -1;
      } // end if 

      this_thread::sleep_for(chrono::milliseconds(1200ms));

      result = ReadLightLevels();
      if(result != 0) {
         PowerOn(false);
         Close();
         return -1;
      } // end if 

      // this status is from the sensor internally,
      unsigned char status = 0;   // 0 = not a valid read
      result = ReadDeviceStatus();
      if(result == 0) {

         // look at lowest bit only, a good read == 1
         status = (GetDeviceStatus()  & 0x01);
      }
      else {
         ret = -1;
      } // end if 
      
      result = PowerOn(false);
      if(result != 0) {
         ret = -1;
      } // end if 

      Close();

      // report the sensor status 
      if(status != TSL2591_VALID_READ_STATUS){
         _error = "sensor didn't complete the read channels";
         ret = -1;
      } // end if 

   } 
   else {
      ret = -1;
   } // end if 

   return ret;
} // end ReadSensor

