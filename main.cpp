

#include "Rp4bPwm.h"
#include <chrono>
#include <thread>
#include "wiringPi.h"
#include "CommonDef.h"

using namespace std;


// g++ -g -opwm main.cpp Rp4bPwm.cpp -lpthread 
int main(int argc, char* args[]){


   wiringPiSetup();                     // initialize wiringPi.  
   pinMode(STEPPER_ENABLE, OUTPUT);     // set STEPPER_ENABLE to output
   pinMode(STEPPER_DIRECTION, OUTPUT);  // set STEPPER_DIRECTION to output

   digitalWrite(STEPPER_ENABLE, LOW);
   digitalWrite(STEPPER_DIRECTION, HIGH);

   unsigned dc = 50;
   Rp4bPwm pwm(PwmNumber::Pwm1);
   pwm.SetFrequenceHz(100);
   pwm.SetDutyCyclePercent(dc);
   pwm.Enable(true);

   digitalWrite(STEPPER_ENABLE, HIGH);
   

   this_thread::sleep_for(chrono::seconds(10));

   // while(dc < 100) {
   //    pwm.SetDutyCyclePercent(dc+=10);
   //    this_thread::sleep_for(chrono::milliseconds(200));
   // } 

   // while(dc > 0) {
   //    pwm.SetDutyCyclePercent(dc-=10);
   //    this_thread::sleep_for(chrono::milliseconds(200));
   // } 

   pwm.Enable(false);

   digitalWrite(STEPPER_ENABLE, LOW);
   digitalWrite(STEPPER_DIRECTION, LOW);

   return 0;
} // end main