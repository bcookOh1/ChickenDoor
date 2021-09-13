

#include "Rp4bPwm.h"
#include <chrono>
#include <thread>
#include "wiringPi.h"
#include "CommonDef.h"

using namespace std;

//g++ -g -opwm main.cpp Rp4bPwm.cpp -lwiringPi -lpthread
int main(int argc, char* args[]){


   wiringPiSetup();                     // initialize wiringPi.  
   pinMode(STEPPER_ENABLE, OUTPUT);     // set STEPPER_ENABLE to output
   pinMode(STEPPER_DIRECTION, OUTPUT);  // set STEPPER_DIRECTION to output

   digitalWrite(STEPPER_ENABLE, HIGH);
   digitalWrite(STEPPER_DIRECTION, HIGH);

   // this_thread::sleep_for(chrono::seconds(10)); // pause

   unsigned dc = 50;
   Rp4bPwm pwm(PwmNumber::Pwm1);
   pwm.SetFrequenceHz(1600);
   pwm.SetDutyCyclePercent(dc);
   pwm.Enable(false);

   size_t tim = 16500;

   for(auto i = 0; i < 1; ++i){

      digitalWrite(STEPPER_DIRECTION, HIGH); // extend
      pwm.Enable(true);
      this_thread::sleep_for(chrono::milliseconds(tim)); // 4650
   
      pwm.Enable(false);
      digitalWrite(STEPPER_DIRECTION, LOW); // retract

      this_thread::sleep_for(chrono::seconds(1));

      pwm.Enable(true);
      this_thread::sleep_for(chrono::milliseconds(tim-50)); // 4750
      pwm.Enable(false);

      this_thread::sleep_for(chrono::seconds(1)); // pause 
   } // end for 

   pwm.Enable(false);
   digitalWrite(STEPPER_ENABLE, LOW);
   digitalWrite(STEPPER_DIRECTION, LOW);

   return 0;
} // end main