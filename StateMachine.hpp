
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
#include "PrintUtils.h"

using namespace std;
namespace sml = boost::sml;


const unsigned MoveUp = 1; 
const unsigned MoveDown = 0;  
// const int On = 1;
// const int Off = 0;


// anonymous namespace 
namespace {

// events 
struct eInit {};
struct eOnTime {};

// for this implementation, states are just empty classes
class Idle1;
class HomingSlowUp;
class HomingUp;
class HomingDown;
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

   // callback to set outputs in main()
   // const string name, const unsigned value
   int SetStateMachineCB(std::function<void(string name, unsigned value)> cb){
      int ret = 0;
      _cb = cb; 
      return ret;
   } // end SetStateMachineCB


   auto operator()()  {
      using namespace sml;

      // guards
      auto AtUp = [this] () -> bool {
         return (_ioValues["up"] == 0);
      }; // end AtUp

      auto AtDown = [this] () -> bool {
         return (_ioValues["down"] == 0);
      }; // end AtDown

      auto Obstructed = [this] () -> bool {
         return (_ioValues["obstructed"] == 0);
      }; // end Obstructed

      auto TimerDone = [this] () -> bool {
         return (_nbTimer.IsDone());
      }; // end TimerDone

      auto IsMorning = [this] () -> bool {
         return (_light >= 1000.0f);
      }; // end IsMorning

      auto IsNight = [this] () -> bool {
         return (_light < 1000.0f);
      }; // end IsNight

      auto ReturnTrue = [] () -> bool {
         return true;
      }; // end ReturnTrue

      return make_transition_table (

         // start homing 
         *state<Idle1> + event<eInit> / [&] {ReadyFlag(false); PrintLn("HomingSlowUp state"); } = state<HomingSlowUp>,
         state<HomingSlowUp> + sml::on_entry<_> / [&] {PrintLn("HomingSlowUp on_entry"); MotorDirection(MoveUp); MotorSpeed(_ac.homingPwmHz); MotorEnable(true); StartTimer(30000);},
         state<HomingSlowUp> + sml::on_exit<_> / [&] {PrintLn("HomingSlowUp on_exit"); MotorEnable(false);},
         state<HomingSlowUp> + event<eOnTime>[AtUp] / [&] {PrintLn("HomingDown state"); KillTimer(); } = state<HomingDown>,
         state<HomingSlowUp> + event<eOnTime>[TimerDone] / [] {PrintLn("Failed1");} = state<Failed>,

         state<HomingDown> + sml::on_entry<_> / [&] {PrintLn("HomingDown on_entry"); MotorDirection(MoveDown); MotorSpeed(_ac.homingPwmHz); MotorEnable(true); StartTimer(2000);},
         state<HomingDown> + sml::on_exit<_> / [&] {PrintLn("HomingDown on_exit"); MotorEnable(false);},
         state<HomingDown> + event<eOnTime>[TimerDone] / [&] {PrintLn("HomingUp state"); KillTimer();} = state<HomingUp>,

         state<HomingUp> + sml::on_entry<_> / [&] {MotorDirection(MoveUp); MotorSpeed(_ac.slowPwmHz); MotorEnable(true); StartTimer(5000);},
         state<HomingUp> + sml::on_exit<_> / [&] {MotorEnable(false); ReadyFlag(true);},
         state<HomingUp> + event<eOnTime>[AtUp] / [&] {PrintLn("Open"); KillTimer();} = state<Open>,
         state<HomingUp> + event<eOnTime>[TimerDone] / [] {PrintLn("Failed2");} = state<Failed>,
         // end homing 

         state<Closed> + sml::on_entry<_> / [&] {MotorEnable(false);},
         state<Closed> + event<eOnTime>[IsMorning] / [] {PrintLn("MovingToOpen");} = state<MovingToOpen>,

         state<MovingToOpen> + sml::on_entry<_> / [&] {MotorDirection(MoveDown); MotorSpeed(_ac.slowPwmHz); MotorEnable(true); StartTimer(7000);},
         state<MovingToOpen> + event<eOnTime>[AtUp] / [&] {PrintLn("Open"); KillTimer();} = state<Open>,
         state<MovingToOpen> + event<eOnTime>[TimerDone] / [] {PrintLn("Failed3");} = state<Failed>,

         state<Open> + sml::on_entry<_> / [&] {MotorEnable(false);},
         state<Open> + event<eOnTime>[IsNight] / [&] {PrintLn("MovingToClose"); KillTimer();} = state<MovingToClose>,
         state<Open> + event<eOnTime>[TimerDone] / [] {PrintLn("Failed4");} = state<Failed>,

         state<MovingToClose> + sml::on_entry<_> / [&] {MotorDirection(MoveUp); MotorSpeed(_ac.slowPwmHz); MotorEnable(true); StartTimer(17000);},
         state<MovingToClose> + event<eOnTime>[AtDown] / [&] {PrintLn("Closed"); KillTimer();} = state<Closed>,
         state<MovingToClose> + event<eOnTime>[TimerDone] / [] {PrintLn("Failed5");} = state<Failed>,

         // obstruction
         state<MovingToClose> + event<eOnTime>[Obstructed] / [&] {PrintLn("ObstructionDetected"); KillTimer();} = state<ObstructionDetected>,
         state<ObstructionDetected> + event<eOnTime>[ReturnTrue] / [] {PrintLn("ObstructionPause"); } = state<ObstructionPause>,
         state<ObstructionPause> + sml::on_entry<_> / [&] {MotorEnable(false); StartTimer(3000);},
         state<ObstructionPause> + event<eOnTime>[TimerDone] / [&] {PrintLn("PauseDone"); KillTimer();} = state<PauseDone>,

         state<PauseDone> + event<eOnTime>[Obstructed] / [] {PrintLn("MovingToOpen");} = state<MovingToOpen>,
         state<PauseDone> + event<eOnTime>[!Obstructed] / [] {PrintLn("MovingToClose");} = state<MovingToClose>
         
      );

   } // end operator() 

private:

   std::function<void(string name, unsigned value)> _cb;
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

   void MotorDirection(unsigned dir) {
      _cb("direction", dir);
      // _ioValues["direction"] = static_cast<unsigned>(dir ? 1 : 0);
   } // end MotorDirection

   void MotorSpeed(int hz) {
      _pwm.SetFrequenceHz(hz);
   } // end MotorSpeed

   void MotorEnable(bool state) {
      _cb("enable", state == true ? 0u : 1u);
      // _ioValues["enable"] = static_cast<unsigned>(state == true ? 0 : 1);
      _pwm.Enable(state);
   }  // end MotorEnable 

   void KillTimer() {
      _nbTimer.Cancel();
   } // end KillTimer

   void StartTimer (unsigned tm) {
      _nbTimer.SetupTimer(tm);
      _nbTimer.StartTimer();
   } // end StartTimer

}; // end struct

} // end anonymous namespace

#endif // end header guard
