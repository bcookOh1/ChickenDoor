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
   
   // cout << _curlCmd << endl;

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

   boost::replace_first(_curlCmd, DATE_PLACEHOLDER, GetNavyFormattedDate());
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

         for (proptree::ptree::value_type& v : tree.get_child(PATH_TO_SUNDATA)) {
            // cout << v.second.get<string>(SUNDATA_ITEM_LABEL) << ", " << v.second.get<string>(SUNDATA_TIME_LABEL) << endl;
            
            if (v.second.get<string>(SUNDATA_ITEM_LABEL) == SUNDATA_RISE_LABEL) {
               _sunriseSunsetStringTimes.sunrise = v.second.get<string>(SUNDATA_TIME_LABEL);
            } // end if 

            if (v.second.get<string>(SUNDATA_ITEM_LABEL) == SUNDATA_SET_LABEL) {
               _sunriseSunsetStringTimes.sunset = v.second.get<string>(SUNDATA_TIME_LABEL);
            } // end if 

            if (v.second.get<string>(SUNDATA_ITEM_LABEL) == SUNDATA_UPPER_TRANSIT_LABEL) {
               _sunriseSunsetStringTimes.upper_transit = v.second.get<string>(SUNDATA_TIME_LABEL);
            } // end if 

         } // end for 

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

   // check that all values are set and nothing missed
   if(_sunriseSunsetStringTimes.sunrise.length() == 0 || 
      _sunriseSunsetStringTimes.sunset.length() == 0 ||
      _sunriseSunsetStringTimes.upper_transit.length() == 0){
      ret = -1;
   } // end if 

   return ret;
} // end ParseResponse

