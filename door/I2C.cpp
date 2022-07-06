/// file: TempHumid.h header for TempHumid class  
/// author: Bennett Cook
/// date: 01-30-2021
/// description: 


#include "I2C.h"


I2C::I2C(unsigned char deviceAddress){
   _bus = "/dev/i2c-1";
   _deviceAddress = deviceAddress;
} // end ctor 

I2C::~I2C(){

} // end dtor 


int I2C::Open(){

   int ret = 0;

   if((_fd = open(_bus.c_str(), O_RDWR)) < 0) {
		_error = "open i2c bus failed";
		return -1;
	} // end if 
   
	// set the I2C device as slave
	ioctl(_fd, I2C_SLAVE, _deviceAddress);

   return ret;
} // end Open

int I2C::Close(){

   int ret = 0;

   if(close(_fd) < 0) {
		_error = "close i2c bus failed";
		return -1;
	} // end if 
   
   return ret;
} // end Close

