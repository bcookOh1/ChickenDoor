#include "PCF8591Reader.h"


PCF8591Reader::PCF8591Reader() {
   _channel = PCF8591_AI_CHANNEL::Ai0;
   _scale = [=](unsigned in){return (in* 1.0f) + 0.0f;};
} // end ctor 


PCF8591Reader::~PCF8591Reader() {
} // end dtor 

void PCF8591Reader::SetChannelAndConversion(PCF8591_AI_CHANNEL channel, Scale scale){
   _channel = _channel; 
   _scale = scale;
} // end SetChannelAndConversion


int PCF8591Reader::RunTask(){
   int ret = 0;
   
   int result = _pcf8591.Read(_channel, _data, _scale);
   if(result == 0) {

      // required call to parent 
      Reader::SetStatus(ReaderStatus::Complete, "no error");
   }
   else {
      Reader::SetStatus(ReaderStatus::Error, _pcf8591.GetErrorStr());
      ret = -1;
   } // end if 

   return ret;
} // end RunTask

float PCF8591Reader::GetData() {
   return _data;
}; // end GetData


