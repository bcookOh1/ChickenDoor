

#include "Rp4bPwm.h"
#include <chrono>
#include <thread>

using namespace std;


// g++ -g -opwm main.cpp Rp4bPwm.cpp -lpthread 
int main(int argc, char* args[]){

   Rp4bPwm pwm(PwmNumber::Pwm1);
   pwm.Reserve(true);
   pwm.SetFrequenceHz(1000);
   pwm.SetDutyCyclePercent(50);
   pwm.Enable(true);

   this_thread::sleep_for(chrono::seconds(10));

   pwm.Enable(false);
   pwm.Reserve(false);

   return 0;
} // end main