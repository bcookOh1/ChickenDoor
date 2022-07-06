
// header guard
#ifndef READER_H
#define READER_H

#include <string>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <boost/atomic.hpp>

using namespace std;


enum class ReaderStatus : int {
   NotStarted = 0,
   Waiting,
   Complete,
   Error
}; // end enum


class Reader {
public:

   Reader();
   virtual ~Reader();

   string GetError() { return _errorStr; }
   ReaderStatus GetStatus(){return _status;}
   void ResetStatus() {_status = ReaderStatus::NotStarted; _errorStr.clear(); }

   virtual int RunTask() = 0;
   int ReadAfterSec(unsigned sec);
   void StopWaiting() { if(ReaderStatus::Waiting == _status) _stopWait = true; }
   void RestartWait() { if(ReaderStatus::Waiting == _status) _restart = true; }

protected:
   void WaitThenRun(unsigned sec);
   void SetStatus(ReaderStatus status, string errorStr);
   void SetError(const string &errorStr);
   
   string _errorStr;
   ReaderStatus _status;

private:
   boost::atomic<bool> _stopWait;
   boost::atomic<bool> _restart;

}; // end class 


#endif // end header guard