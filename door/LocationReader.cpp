#include "LocationReader.h"


LocationReader::LocationReader(const Address address) {
   _curlCmd = CURL_LOCATION_CMD;
   _address.house_number = address.house_number;
   _address.street = address.street;
   _address.city = address.city;
   _address.state = address.state;
   _address.zip_code = address.zip_code;
} // end ctor 

LocationReader::~LocationReader() {
} // end dtor 


int LocationReader::RunTask() {
   int ret = 0;
   ReplacePlaceholders();
   
   _child = unique_ptr<bp::child>(new bp::child(_curlCmd));

   _child->wait();
  
   if(_child->exit_code() == 0) {
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


int LocationReader::ReplacePlaceholders() {
   int ret = 0;

   // build each address item by replacing any spaces with "+"
   string street = _address.house_number + " " + _address.street;
   string city = _address.city;
   string state = _address.state;
   string zip_code = _address.zip_code;

   boost::replace_all(street, " ", "+");
   boost::replace_all(city, " ", "+");
   boost::replace_all(state, " ", "+");
   boost::replace_all(zip_code, " ", "+");

   boost::replace_first(_curlCmd, STREET_PLACEHOLDER, street);
   boost::replace_first(_curlCmd, CITY_PLACEHOLDER, city);
   boost::replace_first(_curlCmd, STATE_PLACEHOLDER, state);
   boost::replace_first(_curlCmd, ZIP_CODE_PLACEHOLDER, zip_code);

   // delete any duplicate "+" chars
   boost::replace_all(_curlCmd, "++", "+");

   return ret;
} // end ReplacePlaceholders


int LocationReader::ParseResponse() {
   int ret = 0;

   proptree::ptree tree;

   try {
      read_json(RESPONSE_FILE, tree);

      // get the lat and lng for the first address then break 
      for (proptree::ptree::value_type& v : tree.get_child("result.addressMatches")) {

         // get the first elements of addressMatches, use try/catch to insure  
         // sure the types are correct
         try {

            _coords.longitude = v.second.get<string>("coordinates.x");
            _coords.latitude = v.second.get<string>("coordinates.y");

         }
         catch (std::exception& e) {
            _errorStr = "lat lng read error, ";
            _errorStr += e.what();
            return -1;
         } // end try catch

         break;
      } // end for 

   }
   catch (std::exception& e) {
      _errorStr = "error on read ";
      _errorStr += e.what();
      ret = -1;
   } // end try/catch

   return ret;
} // end ParseResponse
