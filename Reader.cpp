
#include "Reader.h"

Reader::Reader() {
   _status = ReaderStatus::NotStarted;
   _stopWait = false;
   _restart = false;
} // end ctor 

Reader::~Reader() {
} // end dtor 


void Reader::SetStatus(ReaderStatus status, string errorStr) {
   _status = status;
   _errorStr = errorStr;
} // end SetStatus


void Reader::SetError(const string &errorStr) {
   _status = ReaderStatus::NotStarted; 
   _errorStr = errorStr; 
} // end SetError


// ref: https://www.cplusplus.com/reference/thread/thread/detach/
/// \ret return 0 wait started 
/// \ret return -1 can not start while in waiting
int Reader::ReadAfterSec(unsigned sec) {
   int ret = 0;

   // allow another run to start is NotStarted
   if(ReaderStatus::Waiting != _status) {
      _status = ReaderStatus::Waiting;
      thread([this](unsigned s) {this->WaitThenRun(s); }, sec).detach();
   }
   else {
      ret = 1; // in process
   } // end if 

   return ret;
} // end ReadAfterSec


void Reader::WaitThenRun(unsigned sec) {
   unsigned countDown = sec;

   // count down for 1 second then run RunTask()
   // if sec = 0 the while loop will not run but after countDown will be max unsigned
   while(countDown--) {

      // allow client to trigger a countdown rest34art 
      if(_restart == true) {
         _restart = false;
         countDown = sec;
      } // end if 

      // _stopWait is set from StopWaiting() is true set to false and return 
      // without running RunTask()
      if(_stopWait == true) {
         _stopWait = false;

         _status = ReaderStatus::NotStarted;
         return;
      } // end if 

      this_thread::sleep_for(chrono::seconds(1));
   } // end while
   
   RunTask();
} // end WaitThenRun



