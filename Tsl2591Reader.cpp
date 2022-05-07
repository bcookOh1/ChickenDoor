#include "Tsl2591Reader.h"


Tsl2591Reader::Tsl2591Reader() {
} // end ctor 


Tsl2591Reader::~Tsl2591Reader() {
} // end dtor 


int Tsl2591Reader::RunTask() {
   int ret = 0;

   int result = _sensor.ReadSensor();
   if(result == 0) {

      _sensorData.lightLevel = _sensor.GetLightLevel();
      _sensorData.rawlightLevel = _sensor.GetRawLightLevel(); 

      // required call to parent 
      Reader::SetStatus(ReaderStatus::Complete, "no error");
   }
   else {
      Reader::SetStatus(ReaderStatus::Error, _sensor.GetErrorStr());
      ret = -1;
   } // end if 

   return ret;
} // end RunTask

Tsl2591Data Tsl2591Reader::GetData() {
   return _sensorData;
}; // end GetData


