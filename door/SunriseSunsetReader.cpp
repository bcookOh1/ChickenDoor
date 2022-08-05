#include "SunriseSunsetReader.h"


SunriseSunsetReader::SunriseSunsetReader() {
   _curlCmd = CURL_SUN_RISE_SET_CMD;
   ready = false;
} // end ctor 

SunriseSunsetReader::~SunriseSunsetReader() {
} // end dtor 

void SunriseSunsetReader::SetCoords(const Coordinates& coords) {
   _coords.latitude = coords.latitude;
   _coords.longitude = coords.longitude;
   ReplacePlaceholders();
   ready = true;
} // end SetCoords

int SunriseSunsetReader::RunTask() {
   int ret = 0;
   if (ready == false) {
      _errorStr = "Coordinates not set";
      return -1;
   } // end if 
   
   _child = unique_ptr<bp::child>(new bp::child(_curlCmd));

   _child->wait();

   if (_child->exit_code() == 0) {
      int result = ParseResponse();
      if (result == 0) {

         // required call to parent 
         Reader::SetStatus(ReaderStatus::Complete, "no error");
      }
      else {

         // required call to parent 
         Reader::SetStatus(ReaderStatus::Error, _errorStr);
         ret = -1;
      } // end if 
   }
   else {

      // required call to parent 
      Reader::SetStatus(ReaderStatus::Error, (boost::format{ "curl exit code: %1%" } % _child->exit_code()).str());
      ret = -1;
   } // end if 

   return ret;
} // end RunTask

int SunriseSunsetReader::ReplacePlaceholders() {
   int ret = 0;

   boost::replace_first(_curlCmd, LATITUDE_PLACEHOLDER, _coords.latitude);
   boost::replace_first(_curlCmd, LONGITUDE_PLACEHOLDER, _coords.longitude);

   return ret;
} // end ReplacePlaceholders


int SunriseSunsetReader::ParseResponse() {
   int ret = 0;

   proptree::ptree tree;

   try {
      read_json(SUN_RESPONSE_FILE, tree);

      try {

         _sunriseSunsetStringTimes.sunrise = tree.get<string>("results.sunrise");
         _sunriseSunsetStringTimes.sunset = tree.get<string>("results.sunset");
         _sunriseSunsetStringTimes.day_length = tree.get<string>("results.day_length");

      }
      catch (std::exception& e) {
         _errorStr = "sun rise sun set read error, ";
         _errorStr += e.what();
         return -1;
      } // end try catch

   }
   catch (std::exception& e) {
      _errorStr = "error on read: ";
      _errorStr += e.what();
      ret = -1;
   } // end try/catch


   return ret;
} // end ParseResponse

