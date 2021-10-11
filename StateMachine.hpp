
// header guard
#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

// ref: https://boost-ext.github.io/sml/index.html
// https://boost-ext.github.io/sml/examples.html, hello world
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>
#include <cassert>
#include <vector>
#include <boost/sml.hpp>
#include <boost/mpl/placeholders.hpp>

#include "Rp4bPwm.h"
#include "CommonDef.h"
#include "Util.h"

using namespace std;
namespace sml = boost::sml;


const int Backward = 0; 
const int Forward = 1;  
const int On = 1;
const int Off = 0;


// anonymous namespace 
namespace {

// events 
struct eInit {};
struct eOnTime {};

// for this implementation, states are just empty classes
class Idle1;
class HomingSlowRetract;
class HomingRetract;
class HomingExtend;
class MovingToOpen;
class Open;
class MovingToClose;
class Closed;
class Failed;
class ObstructionDetected;
class ObstructionPause;
class PauseDone;


// transition table
// example: src_state +event<>[guard] / action = dst_state,
// transition from src_state to dst_state on event with guard and action
class sm_chicken_coop {
   using self = sm_chicken_coop;
public:

   explicit sm_chicken_coop(IoValues &ioValues, AppConfig &ac, Rp4bPwm &pwm, NoBlockTimer &nbTimer, float &light) :
      _ioValues(ioValues), _ac(ac), _pwm(pwm), _nbTimer(nbTimer), _light(light) {
         _readyFlag = false;
   } // end ctor 

   auto operator()()  {
      using namespace sml;

      // guards
      auto Retracted = [this] () -> bool {
         return (_ioValues["retracted"] == 0);
      }; // end Retracted

      auto Extended = [this] () -> bool {
         return (_ioValues["extended"] == 0);
      }; // end Extended

      auto Obstructed = [this] () -> bool {
         return (_ioValues["obstructed"] == 0);
      }; // end Obstructed

      auto TimerDone = [this] () -> bool {
         return (_nbTimer.IsDone());
      }; // end TimerDone

      auto IsMorning = [this] () -> bool {
         return (_light >= 150);
      }; // end IsMorning

      auto IsNight = [this] () -> bool {
         return (_light <= 100);
      }; // end IsNight

      auto ReturnTrue = [] () -> bool {
         return true;
      }; // end ReturnTrue


      return make_transition_table (

         // start homing 
         *state<Idle1> + event<eInit> / [&] {ReadyFlag(false);} = state<HomingSlowRetract>,
         state<HomingSlowRetract> + sml::on_entry<_> / [&] {MotorDirection(Backward); MotorSpeed(_ac.homingPwmHz); MotorEnable(true); StartTimer(34000);},
         state<HomingSlowRetract> + sml::on_exit<_> / [&] {MotorEnable(false);},
         state<HomingSlowRetract> + event<eOnTime>[Retracted] / [&] {cout << "HomingExtend" << endl; KillTimer();} = state<HomingExtend>,
         state<HomingSlowRetract> + event<eOnTime>[TimerDone] / [] {cout << "Failed1" << endl;} = state<Failed>,

         state<HomingExtend> + sml::on_entry<_> / [&] {MotorDirection(Forward); MotorSpeed(_ac.slowPwmHz); MotorEnable(true); StartTimer(2000);},
         state<HomingExtend> + sml::on_exit<_> / [&] {MotorEnable(false);},
         state<HomingExtend> + event<eOnTime>[TimerDone] / [&] {cout << "HomingRetract" << endl; KillTimer();} = state<HomingRetract>,

         state<HomingRetract> + sml::on_entry<_> / [&] {MotorDirection(Backward); MotorSpeed(_ac.slowPwmHz); MotorEnable(true); StartTimer(5000);},
         state<HomingRetract> + sml::on_exit<_> / [&] {MotorEnable(false); ReadyFlag(true);},
         state<HomingRetract> + event<eOnTime>[Retracted] / [&] {cout << "Closed" << endl; KillTimer();} = state<Closed>,
         state<HomingRetract> + event<eOnTime>[TimerDone] / [] {cout << "Failed2" << endl;} = state<Failed>,
         // end homing 

         state<Closed> + sml::on_entry<_> / [&] {MotorEnable(false);},
         state<Closed> + event<eOnTime>[IsMorning] / [] {cout << "MovingToOpen" << endl;} = state<MovingToOpen>,

         state<MovingToOpen> + sml::on_entry<_> / [&] {MotorDirection(Forward); MotorSpeed(_ac.fastPwmHz); MotorEnable(true); StartTimer(7000);},
         state<MovingToOpen> + event<eOnTime>[Extended] / [&] {cout << "Open" << endl; KillTimer();} = state<Open>,
         state<MovingToOpen> + event<eOnTime>[TimerDone] / [] {cout << "Failed3" << endl;} = state<Failed>,

         state<Open> + sml::on_entry<_> / [&] {MotorEnable(false);},
         state<Open> + event<eOnTime>[IsNight] / [&] {cout << "MovingToClose" << endl; KillTimer();} = state<MovingToClose>,
         state<Open> + event<eOnTime>[TimerDone] / [] {cout << "Failed" << endl;} = state<Failed>,

         state<MovingToClose> + sml::on_entry<_> / [&] {MotorDirection(Backward); MotorSpeed(_ac.slowPwmHz); MotorEnable(true); StartTimer(17000);},
         state<MovingToClose> + event<eOnTime>[Retracted] / [&] {cout << "Closed" << endl; KillTimer();} = state<Closed>,
         state<MovingToClose> + event<eOnTime>[TimerDone] / [] {cout << "Failed4" << endl;} = state<Failed>,

         // obstruction
         state<MovingToClose> + event<eOnTime>[Obstructed] / [&] {cout << "ObstructionDetected" << endl; KillTimer();} = state<ObstructionDetected>,
         state<ObstructionDetected> + event<eOnTime>[ReturnTrue] / [] {cout << "ObstructionPause" << endl; } = state<ObstructionPause>,
         state<ObstructionPause> + sml::on_entry<_> / [&] {MotorEnable(false); StartTimer(3000);},
         state<ObstructionPause> + event<eOnTime>[TimerDone] / [&] {cout << "PauseDone" << endl; KillTimer();} = state<PauseDone>,

         state<PauseDone> + event<eOnTime>[Obstructed] / [] {cout << "MovingToOpen" << endl;} = state<MovingToOpen>,
         state<PauseDone> + event<eOnTime>[!Obstructed] / [] {cout << "MovingToClose" << endl;} = state<MovingToClose>
         
      );

   } // end operator() 

private:

   IoValues &_ioValues;
   AppConfig &_ac;
   Rp4bPwm &_pwm;
   NoBlockTimer &_nbTimer;
   float &_light;
   bool _readyFlag;

   // actions
   void ReadyFlag(bool state) {
      _readyFlag = state;
   } // end ReadyFlag

   void MotorDirection(int dir) {
      _ioValues["direction"] = (dir ? 1 : 0);
      cout << _ioValues["direction"] << ", from " << dir << endl;
   } // end MotorDirection

   void MotorSpeed(int hz) {
      _pwm.SetFrequenceHz(hz);
   } // end MotorSpeed

   void MotorEnable(bool state) {
      _pwm.Enable(state);
      _ioValues["enable"] = (state ? 1 : 0);
   }  // end MotorEnable 

   void KillTimer() {
      _nbTimer.Cancel();
   } // end KillTimer

   void StartTimer (unsigned tm) {
      _nbTimer.SetupTimer(tm);
      _nbTimer.StartTimer();
   } // end StartTimer

   void SetMovingLed(bool state) { 
      _ioValues["door_cycling"] = (state ? 1 : 0); 
   } // end SetMovingLed

}; // end struct

} // end anonymous namespace

#endif // end header guard
