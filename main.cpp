

#include "Rp4bPwm.h"
#include <chrono>
#include <thread>

using namespace std;


// g++ -g -opwm main.cpp Rp4bPwm.cpp -lpthread 
int main(int argc, char* args[]){

   unsigned dc = 10;
   Rp4bPwm pwm(PwmNumber::Pwm1);
   pwm.SetFrequenceHz(1000);
   pwm.SetDutyCyclePercent(dc);
   pwm.Enable(true);


   while(dc < 100) {
      pwm.SetDutyCyclePercent(dc+=10);
      this_thread::sleep_for(chrono::milliseconds(200));
   } 

   while(dc > 0) {
      pwm.SetDutyCyclePercent(dc-=10);
      this_thread::sleep_for(chrono::milliseconds(200));
   } 

   return 0;
} // end main