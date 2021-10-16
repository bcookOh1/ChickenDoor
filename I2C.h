/// file: I2C.h header for I2C base class  
/// author: Bennett Cook
/// date: 01-31-2021
/// description: 


// header guard
#ifndef I2C_H
#define I2C_H

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <fcntl.h>

#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>


using namespace std;

class I2C {
public:
   I2C(unsigned char deviceAddress);
   virtual ~I2C();

   int Open();
   int Close();

   string GetErrorStr() {return _error;}

 protected:

	int _fd;
   string _error;
   string _bus;
   unsigned char _deviceAddress;

}; // end class

#endif  // end header guard
