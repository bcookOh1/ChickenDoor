#include "Si7021Reader.h"


Si7021Reader::Si7021Reader() {
} // end ctor 


Si7021Reader::~Si7021Reader() {
} // end dtor 


int Si7021Reader::RunTask() {
   int ret = 0;

   int result = _sensor.ReadSensor(SI7021_READINGS::Both);
   if(result == 0) {

      _sensorData.temperature = _sensor.TempToString(false);
      _sensorData.TemperatureUnits = _sensor.GetTemperatureUnits(false); 
      _sensorData.humidity = _sensor.HumidityToString();
      _sensorData.humidityUnits = _sensor.GetHumidityUnits();

      // required call to parent 
      Reader::SetStatus(ReaderStatus::Complete, "no error");
   }
   else {
      Reader::SetStatus(ReaderStatus::Error, _sensor.GetErrorStr());
      ret = -1;
   } // end if 

   return ret;
} // end RunTask

Si7021Data Si7021Reader::GetData() {
   return _sensorData;
}; // end GetData


