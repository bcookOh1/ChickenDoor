
/// file: PCF8591.h header for PCF8591, analog IO using a the I2C base class  
/// author: Bennett Cook
/// date: 10-14-2021
/// description: 
/// ref: https://www.nxp.com/docs/en/data-sheet/PCF8591.pdf
/// ref: https://stackoverflow.com/questions/7787500/how-to-write-a-function-that-takes-a-functor-as-an-argument

// header guard

#ifndef PCF8591_H
#define PCF8591_H

#include "I2C.h"

#include <functional>

// alias for Read(). this is a conversion function to convert  
// the ad counts to a meaningful value
using Scale = std::function<float(unsigned char)>;


// PCF8591 I2C device address 
const unsigned char PCF8591_I2C_ADDRESS = 0x48;

enum class PCF8591_AI_CHANNEL : unsigned char {
   Ai0 = 0,
   Ai1,
   Ai2,
   Ai3
};  // end enum


class PCF8591 : public I2C {
public:
   PCF8591();
   ~PCF8591();
   
   int Read(PCF8591_AI_CHANNEL channel, float &output, Scale scale);
   
private:

}; // end class

#endif  // end header guard