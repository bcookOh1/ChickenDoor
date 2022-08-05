
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "CommonDef.h"
#include "Reader.h"

using namespace std;
using namespace boost;
namespace proptree = boost::property_tree;
namespace bp = boost::process;



// search and replace placeholders in url, note square brackets "[]" are not legal chars in urls
const string LATITUDE_PLACEHOLDER = "[lat]";  // float as a string with n decimel places 
const string LONGITUDE_PLACEHOLDER = "[lng]"; // float as a string with n decimel places

const string SUN_RESPONSE_FILE = "sun.json";

// ref for timeouts: https://stackoverflow.com/questions/42873285/curl-retry-mechanism
// curl -o sun_rise_sun_set.json "https://api.sunrise-sunset.org/json?lat=42.1867052&lng=-86.2605776"
const string CURL_SUN_RISE_SET_CMD = "curl -o " + SUN_RESPONSE_FILE
                                   + " \"http://api.sunrise-sunset.org/json?lat=[lat]&lng=[lng]\"";


class SunriseSunsetReader : public Reader {
public:

   SunriseSunsetReader();
   virtual ~SunriseSunsetReader();

   int RunTask() override;
   void SetCoords(const Coordinates& coords);
   SunriseSunsetStringTimes GetData() { return _sunriseSunsetStringTimes; }

private:

   int ReplacePlaceholders();
   int ParseResponse();

   Coordinates _coords;
   SunriseSunsetStringTimes _sunriseSunsetStringTimes;
   string _curlCmd;
   bool ready;

   unique_ptr<bp::child> _child;

}; // end class

/*
{
   "results": {
      "sunrise":"10:07:50 AM",
      "sunset":"1:26:50 AM",
      "solar_noon":"5:47:20 PM",
      "day_length":"15:19:00",
      "civil_twilight_begin":"9:34:53 AM",
      "civil_twilight_end":"1:59:46 AM",
      "nautical_twilight_begin":"8:50:10 AM",
      "nautical_twilight_end":"2:44:30 AM",
      "astronomical_twilight_begin":"7:55:47 AM",
      "astronomical_twilight_end":"3:38:53 AM"
   },
   "status":"OK"
}


*/

