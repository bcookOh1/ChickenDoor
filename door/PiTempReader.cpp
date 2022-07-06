
#include "PiTempReader.h"


PiTempReader::PiTempReader() {

} // end ctor 

PiTempReader::~PiTempReader() {

} // end dtor 

int PiTempReader::RunTask() {
   int ret = 0;

      // read the temp once at the start so the temperature var is valid
   int result = ReadBoardTemperature(_temperature);
   if(result != 0){

      // required call to parent 
      Reader::SetStatus(ReaderStatus::Error, "board temperature read failed ");
      ret = -1;
   }
   else {

      // required call to parent 
      Reader::SetStatus(ReaderStatus::Complete, "no error");
   
   } // end if 

   return ret;
} // end ReadAsnyc

string PiTempReader::GetData() {
   return _temperature;
} // end GetData



