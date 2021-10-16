
#include "PCF8591.h"

PCF8591::PCF8591() : I2C(PCF8591_I2C_ADDRESS) { 
} // end ctor 

PCF8591::~PCF8591(){
} // end dtor 
   
   
int PCF8591::Read(PCF8591_AI_CHANNEL channel, float &output, Scale scale){
   int ret = 0;
   const unsigned cmd = 0x40;
   unsigned char data[1] = {0};

   int result = Open();
   if(result == 0) {

      for(int i = 0; i < 2; i++) {
      // write/read twice based on WiringPI PCF8591 library source
         
         data[0] = (cmd + static_cast<unsigned char>(channel));
         if(write(_fd, data, 1) != 1) {
            _error = "PCF8591 write error";
            ret = -1;
            break;
         } // end if 
            
         data[0] = 0;   
         if(read(_fd, data, 1) != 1) {
            _error = "PCF8591 read error";
            ret = -1;
            break;
         } // end if 
      
      } // end for 
      
      result = Close();
      if(result != 0) {
         _error = "PCF8591 close error";
         ret = -1;
      } // end if 
   
   }
   else {
      _error = "PCF8591 open error";      
      ret = -1;
   } // end if 

   // if no error set output 
   if(ret == 0) {
      _error.clear();
      output = scale(data[0]);
   } // end if   
   
   return ret;
} // end Read
