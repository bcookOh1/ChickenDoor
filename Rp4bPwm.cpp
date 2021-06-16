
#include "Rp4bPwm.h"

Rp4bPwm::Rp4bPwm(PwmNumber pwmNum){
   _pwmNum = pwmNum;
   _reserved = false;
   _enabled = false;
   _period = 0;
   _dutyCycle = 0;

   _exportFilePath = "//sys//class//pwm//pwmchip0//export";
   _unexportFilePath = "//sys//class//pwm//pwmchip0//unexport";
   _polarityFilePath = "//sys//class//pwm//pwmchip0//pwm0/polarity";
   _periodFilePath = "//sys//class//pwm//pwmchip0//pwm0//period";
   _dutyCycleFilePath = "//sys//class//pwm//pwmchip0//pwm0//duty_cycle";
   _enableFilePath = "//sys//class//pwm//pwmchip0//pwm0//enable";

} // end ctor 


Rp4bPwm::~Rp4bPwm(){

} // end dtor 


/// ret -1 = substr not found 
/// ret 0 = success
/// ret 1 = no change needed
int Rp4bPwm::SetPwmNumInPath(string &path){
   int ret = 0;

   string currentPwm = _PwmNumStrings[_pwmNum];
   string otherPwm = (_pwmNum == PwmNumber::Pwm0 ? _PwmNumStrings[0] : _PwmNumStrings[1]);

    // the find_first() return iterator_range converts to a bool
    if(find_first(path, currentPwm) {
        ret = 1;
    }
    else {
      replace_first(path, otherPwm, currentPwm);
    } // end if 
        
   return ret;
} // end SetPwmNumInPath


int Rp4bPwm::WriteFile(const string &path, const string &value){
      int ret = 0

      fstream file;
      file.open(path, ios::out);
      if(!file) {
         ret = -1;
      }
      else {
         file << value;
         file.close(); 
      } // end if 

      return ret;
   } // end WriteFile


int Rp4bPwm::Reserve(bool state){
   int ret = 0;
   int result = 0;
   
   if(state == true){

      if(_reserved == true) {
         _errStr = "pwm resource is already reserved";
         return -1;
      } // end if 

      SetPwmNumInPath(_polarityFilePath); 
      SetPwmNumInPath(_periodFilePath);
      SetPwmNumInPath(_dutyCycleFilePath);
      SetPwmNumInPath(_enableFilePath);

      result = WriteFile(_exportFilePath, lexical_cast<string>(_pwmNum)):
      _reserved = true;
   }
   else {

      if(_reserved == false) {
         _errStr = "pwm resource is already released";
         return -1;
      } // end if 

      result = WriteFile(_unexportFilePath, lexical_cast<string>(_pwmNum)):
      _reserved = false;
   } // end if 

   // for debug 
   assert(result == 0)

   return ret;
} // end Reserve


int Rp4bPwm::Enable(bool state){
   int ret = 0;

   if(_reserved != true) {
      _errStr = "no pwm resource is reserved";
      return -1;
   } // end if 

   if(state == true && _enabled == true){
      _errStr = "pwm is already enabled";
      return -1;
   }
   else if(state == false && _enabled == false){
      _errStr = "pwm is already disabled";
      return -1;
   }
   else if(state == true && _enabled == false){
      result = WriteFile(_enableFilePath, "1");
      _enabled = true;
   }
   else if(state == false && _enabled == true){
      result = WriteFile(_enableFilePath, "0");
      _enabled = false;
   } // end if 

   // for debug 
   assert(result == 0)

   return ret;
} // end Enable

/// param  hz [1:10000]
int Rp4bPwm::SetFrequenceHz(unsigned hz){
   int ret = 0;

   if(_reserved != true) {
      _errStr = "no pwm resource is reserved";
      return -1;
   } // end if 

   if(_enabled == true) {
      _errStr = "disable before setting frequency";
      return -1;
   } // end if 

   // range check
   if(hz == 0 || hz > 10000){
      _errStr = "hz is out of range[1:10000]";
      return -1;
   } // end if 

   _period = static_cast<unsigned>(NanoSecIn1Second * (1.0/hz));
   int result = WriteFile(_periodFilePath, lexical_cast<string>(_period));

   // for debug 
   assert(result == 0)

   return ret;
} // end SetFrequenceHz


int Rp4bPwm::SetDutyCyclePercent(unsigned dc){
   int ret = 0;

   if(_reserved != true) {
      _errStr = "no pwm resource is reserved";
      return -1;
   } // end if 

   // range check
   if(hz == 0 || hz > 100){
      _errStr = "duty cycle is out of range[1:100]";
      return -1;
   } // end if 

   _dutyCycle = static_cast<unsigned>(_period * (dc/100.0));
   int result = WriteFile(_dutyCycleFilePath, lexical_cast<string>(_dutyCycle));

   // for debug 
   assert(result == 0)

   return ret;
} // end SetDutyCyclePercent


