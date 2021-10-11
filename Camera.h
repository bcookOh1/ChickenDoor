
#ifndef CAMERA_H
#define CAMERA_H

#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
// #include <boost/timer/timer.hpp>

#include <string>
#include <iostream>
#include <thread>
#include <future>

#include "Util.h"

using namespace std;
namespace bp = boost::process;

class Camera {
public:

   Camera();
   ~Camera(); 

   // ret 0 success
   // ret 1 in process 
   // ret -1 failed, error string set
   int GetStatus() { return _status; }
   string GetError() { return _errorStr; }

   void StillAsync();
   bool IsDone();

private:

   string _filename;
   string _errorStr;
   int _status;
   future<int> _fut;

}; // end class

int TakeStillTask(const string &cmdString);

#endif // end guard 