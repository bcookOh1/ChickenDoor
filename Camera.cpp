
#include "Camera.h"
#include "PrintUtils.h"

Camera::Camera(){
   _status = 0;
} // end ctor 

Camera::~Camera(){
} // end dtor 


void Camera::StillAsync() {

   std::string fname = "Obstruction_";
   fname += GetDateTimeFilename();
   fname += ".jpg";
   _filename = fname;
   
   string cmdString = "raspistill -o # -ex auto -awb auto -vf -hf -q 75 -n -t 800";
   boost::replace_first(cmdString, "#", _filename); 

   _fut = async(launch::async, TakeStillTask, cmdString);

} // end StillAsync


bool Camera::IsDone(){
   bool ret = false;

   if(_fut.wait_for(0ms) == future_status::timeout) {
      ret = false;
      PrintLn((boost::format{ "future_status::timeout, status: %1%" } % _status).str());
   }
   else if (_fut.wait_for(0ms) == future_status::ready) {
      _status = _fut.get();
      ret = true;
      PrintLn((boost::format{ "future_status::timeout, status: %1%" } % _status).str());
   }
   else {
      _status = _fut.get();
      ret = true;
      PrintLn((boost::format{ "future_status::timeout, status: %1%" } % _status).str());
   } // end if 

   return ret;
} // end IsDone

int TakeStillTask(const string &cmdString) {
   int ret = bp::system(cmdString.c_str());
   return ret;
} // end TakeStillTask




