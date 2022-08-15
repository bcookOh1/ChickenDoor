
#include "Camera.h"
#include "PrintUtils.h"

Camera::Camera(){
   _status = 0;
} // end ctor 

Camera::~Camera(){
} // end dtor 


void Camera::StillAsync() {

   std::string fname = "/var/www/html/pics/coop.jpg";
   _filename = fname;
   
   string cmdString = "raspistill -o # -ex auto -awb auto -q 75 -n -rot 270";
   boost::replace_first(cmdString, "#", _filename); 

   _fut = async(launch::async, TakeStillTask, cmdString);

} // end StillAsync


bool Camera::IsDone(){
   bool ret = false;

   if(_fut.wait_for(0ms) == future_status::timeout) {
      ret = false;
      PrintLn((boost::format{ "camera, future timeout: %1%" } % _status).str());
   }
   else if (_fut.wait_for(0ms) == future_status::ready) {
      _status = _fut.get();
      ret = true;
      PrintLn((boost::format{ "camera, future ready: %1%" } % _status).str());
   }
   else {
      _status = _fut.get();
      ret = true;
      PrintLn((boost::format{ "camera, future differed: %1%" } % _status).str());
   } // end if 

   return ret;
} // end IsDone

int TakeStillTask(const string &cmdString) {
   int ret = bp::system(cmdString.c_str());
   return ret;
} // end TakeStillTask




