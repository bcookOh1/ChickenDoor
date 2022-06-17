
// file utils.h
// author: BCook
// date: 07/06/2020 
// description: header file for the sql utilites 


// header guard
#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <numeric> 
#include <cassert>
#include <type_traits>

#include "CommonDef.h"
#include "UpdateDatabase.h"
#include "PrintUtils.h"

using namespace std::chrono_literals;
using MsDuration = std::chrono::duration<int, std::ratio<1, 1000>>;
using namespace std;

// return a date string in standard format "YYYY-MM-DD HH:MM:SS" with  the current date and time  
string GetSqlite3DateTime();

// return a date time string in format "YYYY-MM-DD HH-MM-SS" with the current date and time 
string GetDateTimeFilename();

// 
IoValues MakeIoValuesMap(const vector<IoConfig> &io);
void PrintIo(const IoValues &ioValues);
string IoToString(const IoValues &ioValues);
string IoToLine(const IoValues &ioValues);

int ReadBoardTemperature(string &temperature);
void UpdateDoorStateDB(DoorState ds, UpdateDatabase &udb, string &light, string &temperature);


// conditional print  with optional newline
// used with command line SilentFlag 
// use string str param instead of const string & to accept literal strings  
inline void CondPrint(string_view strv, bool cond, bool newline = false) {
   if(cond){
      cout << strv << (newline ? '\n' : ' ');
      cout.flush();
   } // end if 
} // end CondPrint


// for use in a loop to detect when a var makes a 
// transition (edge detect)
// for now it only compiles with int or float types
template<typename T>
class OneShot {
public:
   OneShot(T value) : _value{value} {
      
      // check that only ints or floats are used
      // so the compare and assignment in Changed() works
      if(!is_integral_v<T> && !is_floating_point_v<T> ) {
         assert(false);
      } // end if 

   } // end ctor

   bool Changed(T value) {

      // look for changed value
      if(value != _value) {
         _value = value;
         return true;
      } // end if 

      return false;
   } // end Changed

private:
   T _value;

}; // end class



// need a better implementation, this works but will be better with boost::asio 
class NoBlockTimer {
public:

   NoBlockTimer() {
      _msd = MsDuration{0};
      _done = false;
      _running = false;
      _kill = false;
   } // end ctor 

   ~NoBlockTimer() {
   } // end dtor 

   // return 0 success
   // return -1 timer is already running
   int SetupTimer(unsigned msd){
      int ret = 0;

      if(_running == false) {
         _msd = MsDuration{msd};
         _done = false;
         _kill = false;
      }
      else {
         ret = -1;
      } // end if 

      PrintLn((boost::format{ "SetupTimer: ret=%1% tm: %2%" } % ret % msd).str());
      return ret;
   } // end SetupTimer


   // return 0 success
   // return -1 timer is already running
   int StartTimer(){
      int ret = 0;

      if(_running == false) {
         _running = true;
         std::thread th([=]() {this->TimerTask(); });
         th.detach();
      }
      else {
         ret = -1;
      } // end if 

      PrintLn((boost::format{ "StartTimer: ret=%1%" } % ret).str());
      return ret;
   } // end StartTimer

   // returns timer done status 
   // remark if time is done returns true and then resets internal done status 
   bool IsDone() {
      bool ret = _done;

      if(_done == true) {
         _done = false;
      } // end if 

      return ret; 
   } // end IsDone

   // cancels the timer (if running) and resets internal done status 
   void Cancel() {
      _kill = true;
      std::this_thread::sleep_for(25ms);
      _done = false;
      _running = false;
      return;
   } // end Cancel

private:

   MsDuration _msd;
   bool _done;
   bool _running;
   bool _kill;

   void TimerTask() {
      MsDuration tc = 0ms;

      PrintLn((boost::format{ "TimerTask: start: ms: %1%" } % _msd.count()).str());
      
      while(!_done){
         
         if(_kill) break;

         std::this_thread::sleep_for(25ms);
         tc += 25ms;
      
         if(tc >= _msd) {
            _done = true;
         } // end if 

         if(_kill) break;

      } // end while 

      PrintLn((boost::format{ "TimerTask: done: %1%" } % _done).str());

      _running = false;
      _kill = false;

      return;
   } // end TimerTask

}; // end class 

template<typename T>
class SmoothingFilter {
public:
   
   SmoothingFilter(size_t length) {
      _length = length;
   } // end ctor  

   ~SmoothingFilter() {
       _queue.clear();
   } // end dtor

   // Add will fill the queue with value 
   // if the queue is not full 
   void Add(T value) {

      // add single value 
      if(_queue.size() == _length){
         _queue.push_back(value);
      }
      else {

         // fill partially full queue with value
         while( _queue.size() < _length)  
            _queue.push_back(value);

      } // end if 
      
      // maintain the queue size, use a while just to be safe 
      while( _queue.size() > _length) _queue.pop_front();
      
      return;
   } // end Add 

   void Clear() { _queue.clear();}
   bool IsReady(){return _queue.size() == _length;} 

   T GetFilteredValue() {
      return accumulate(_queue.begin(),_queue.end(), 0.0f) / _queue.size();
   } // end GetAverage

private:
   size_t _length;
   deque<T> _queue;
}; // end class


#endif // end header guard
