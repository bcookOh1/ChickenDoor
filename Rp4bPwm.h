
// file Rp4bPwm.h
// author: BCook
// date: 06/16/2021 
// description: header file for the sql utilites 


// header guard
#ifndef RP4BPWM_H
#define RP4BPWM_H

#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


using namespace std;
using namespace boost;


enum class PwmNumber : int {
   Pwm0 = 0,
   Pwm1
}; // end enum

const double NanoSecIn1Second = 1.0E+09;

//  in export use 0 (pwm0) for pin 18 and 1 (pwm1) for pin 19  


class Rp4bPwm {
public:

   Rp4bPwm(PwmNumber pwmNum);
   ~Rp4bPwm();

   int SetFrequenceHz(unsigned hz);
   int SetDutyCyclePercent(unsigned dc);
   int Enable(bool state);

   string GetErrStr(){return _errStr;}

private:

   PwmNumber _pwmNum;
   bool _reserved;
   bool _enabled;
   unsigned _period;
   unsigned _dutyCycle;
   string _errStr;

   string _exportFilePath;
   string _unexportFilePath;
   string _polarityFilePath;
   string _periodFilePath;
   string _dutyCycleFilePath;
   string _enableFilePath;

   const vector<string> _PwmNumStrings{{"pwm0"},{"pwm1"}};

   int Reserve(bool state);
   int SetPwmNumInPath(string &path);
   int WriteFile(const string &path, const string &value);

}; // end class

#endif // end header guard