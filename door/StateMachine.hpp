
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

enum class DoorCommand : int {
   NoChange = 0,
   Open,
   Close,
}; // end enum


// anonymous namespace 
namespace {

// events 
struct eInit {};
struct eStartUp {};
struct eOnTime {
   DoorCommand dc{DoorCommand::NoChange};
}; // end struct

// for this implementation, states are just empty classes
class Idle1;
class HomingSlowUp;
class HomingUp;
class HomingDown;
class MovingToOpen;
class HomingComplete;
class Open;
class MovingToClose;
class ClosedLock;
class Closed;
class Failed;
class ObstructionDetected;
class ObstructionPause;
class PauseDone;

class EnterManual;
class ManualMovingToOpen;
class ManualOpen;
class ManualMovingToClose;
class ManualClosed;
class ExitManual;


// transition table
// example: src_state +event<>[guard] / action = dst_state,
// transition from src_state to dst_state on event with guard and action
class sm_chicken_coop {
   using self = sm_chicken_coop;
public:

   explicit sm_chicken_coop(IoValues &ioValues, AppConfig &ac, Rp4bPwm &pwm, NoBlockTimer &nbTimer) :
      _ioValues(ioValues), _ac(ac), _pwm(pwm), _nbTimer(nbTimer) {
   } // end ctor 

   // callback to set outputs in main()
   // const string name, const unsigned value
   int SetStateMachineCB(std::function<void(DoorState ds)> cb){
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

      auto IsDay = [this] (const eOnTime &e) -> bool {
         return (e.dc == DoorCommand::Open);
      }; // end IsDay

      auto IsNight = [this] (const eOnTime &e) -> bool {
         return (e.dc == DoorCommand::Close);
      }; // end IsNight

      auto ReturnTrue = [] () -> bool {
         return true;
      }; // end ReturnTrue

      return make_transition_table (

         // start homing 
         *state<Idle1> + event<eInit> / [&] { PrintLn("HomingSlowUp state"); } = state<HomingSlowUp>,
         state<HomingSlowUp> + sml::on_entry<_> / [&] {PrintLn("HomingSlowUp on_entry");  _cb(DoorState::Startup); MotorDirection(MoveUp); MotorSpeed(_ac.pwmHzHoming); MotorEnable(true); StartTimer(30000);},
         state<HomingSlowUp> + sml::on_exit<_> / [&] {PrintLn("HomingSlowUp on_exit"); MotorSpeed(0); },
         state<HomingSlowUp> + event<eStartUp>[AtUp] / [&] {PrintLn("HomingDown state"); KillTimer(); } = state<HomingDown>,
         state<HomingSlowUp> + event<eStartUp>[TimerDone] / [&] {PrintLn("Failed1");  MotorSpeed(0);} = state<Failed>,

         state<HomingDown> + sml::on_entry<_> / [&] {PrintLn("HomingDown on_entry"); MotorDirection(MoveDown); MotorSpeed(_ac.pwmHzHoming); StartTimer(1500);},
         state<HomingDown> + sml::on_exit<_> / [&] {PrintLn("HomingDown on_exit"); MotorSpeed(0); },
         state<HomingDown> + event<eStartUp>[TimerDone] / [&] {PrintLn("HomingUp state"); KillTimer();} = state<HomingUp>,

         state<HomingUp> + sml::on_entry<_> / [&] { MotorDirection(MoveUp); MotorSpeed(_ac.pwmHzSlow); StartTimer(2000);},
         state<HomingUp> + sml::on_exit<_> / [&] {MotorSpeed(0); },
         state<HomingUp> + event<eStartUp>[AtUp] / [&] {PrintLn("HomingComplete"); MotorSpeed(0); KillTimer();} = state<HomingComplete>,
         state<HomingUp> + event<eStartUp>[TimerDone] / [&] {PrintLn("Failed2");  MotorSpeed(0);} = state<Failed>,
         state<HomingComplete> + event<eOnTime>[ReturnTrue] / [&] {PrintLn("Open from HomingComplete");  MotorSpeed(0);} = state<Open>, 
         // note the eOnTime is sent for HomingComplete when light data is available
         // end homing 

         // normal sequence 
         state<Closed> + sml::on_entry<_> / [&] {_cb(DoorState::Closed); MotorSpeed(0); },
         state<Closed> + event<eOnTime>[IsDay] / [] {PrintLn("MovingToOpen");} = state<MovingToOpen>,

         state<MovingToOpen> + sml::on_entry<_> / [&] {_cb(DoorState::MovingToOpen); MotorDirection(MoveUp); MotorSpeed(_ac.pwmHzFast); StartTimer(30000);},
         state<MovingToOpen> + event<eOnTime>[AtUp] / [&] {PrintLn("Open"); KillTimer();} = state<Open>,
         state<MovingToOpen> + event<eOnTime>[TimerDone] / [&] {PrintLn("Failed3");  MotorSpeed(0);} = state<Failed>,

         state<Open> + sml::on_entry<_> / [&] {_cb(DoorState::Open); MotorSpeed(0); },
         state<Open> + event<eOnTime>[IsNight] / [&] {PrintLn("MovingToClose"); KillTimer();} = state<MovingToClose>,
         state<Open> + event<eOnTime>[TimerDone] / [&] {PrintLn("Failed4");  MotorSpeed(0);} = state<Failed>,

         state<MovingToClose> + sml::on_entry<_> / [&] {_cb(DoorState::MovingToClose); MotorDirection(MoveDown); MotorSpeed(_ac.pwmHzSlow); StartTimer(30000);},
         state<MovingToClose> + event<eOnTime>[AtDown] / [&] {PrintLn("ClosedLock"); KillTimer(); StartTimer(1500);} = state<ClosedLock>,
         state<ClosedLock> + event<eOnTime>[TimerDone] / [&] {PrintLn("Closed"); KillTimer();} = state<Closed>,
         state<MovingToClose> + event<eOnTime>[TimerDone] / [&] {PrintLn("Failed5"); MotorSpeed(0);} = state<Failed>,

         // obstruction
         state<MovingToClose> + event<eOnTime>[Obstructed] / [&] {PrintLn("ObstructionDetected"); KillTimer();} = state<ObstructionDetected>,
         state<ObstructionDetected> + event<eOnTime>[ReturnTrue] / [] {PrintLn("ObstructionPause"); } = state<ObstructionPause>,
         state<ObstructionPause> + sml::on_entry<_> / [&] {_cb(DoorState::Obstructed); MotorSpeed(0); StartTimer(3000);},
         state<ObstructionPause> + event<eOnTime>[TimerDone] / [&] {PrintLn("PauseDone"); KillTimer();} = state<PauseDone>,

         state<PauseDone> + event<eOnTime>[Obstructed] / [] {PrintLn("MovingToOpen");} = state<MovingToOpen>,
         state<PauseDone> + event<eOnTime>[!Obstructed] / [] {PrintLn("MovingToClose");} = state<MovingToClose>

      );

   } // end operator() 

private:

   std::function<void(DoorState ds)> _cb;
   IoValues &_ioValues;
   AppConfig &_ac;
   Rp4bPwm &_pwm;
   NoBlockTimer &_nbTimer;

   void MotorDirection(unsigned dir) {
      _ioValues["direction"] = static_cast<unsigned>(dir ? 1 : 0);
   } // end MotorDirection

   void MotorSpeed(int hz) {

      // use hz to enable/disable the pwn output 
      if(hz > 0){
         _pwm.SetFrequenceHz(hz);
         _pwm.Enable(true);
      }
      else {
         _pwm.Enable(false);
      } // end if

      PrintLn((boost::format{ "motor speed: %d" } %  hz).str());
   } // end MotorSpeed

   void MotorEnable(bool state) {
      _ioValues["enable"] = static_cast<unsigned>(state == true ? 0 : 1);
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
