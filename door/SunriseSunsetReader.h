
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "CommonDef.h"
#include "DateTimeUtils.h"
#include "Reader.h"

using namespace std;
using namespace boost;
namespace proptree = boost::property_tree;
namespace bp = boost::process;

// json path and labels to parse the response
const string PATH_TO_SUNDATA = "properties.data.sundata";
const string SUNDATA_ITEM_LABEL = "phen";
const string SUNDATA_TIME_LABEL = "time";
const string SUNDATA_RISE_LABEL = "Rise";
const string SUNDATA_SET_LABEL = "Set";
const string SUNDATA_UPPER_TRANSIT_LABEL = "Upper Transit";


// search and replace placeholders in url, note square brackets "[]" are not legal chars in urls
const string LATITUDE_PLACEHOLDER = "[lat]";  // float as a string with n decimel places 
const string LONGITUDE_PLACEHOLDER = "[lng]"; // float as a string with n decimel places
const string DATE_PLACEHOLDER = "[date]";     // yyyy-mm-dd format 

const string SUN_RESPONSE_FILE = "sun.json";

// ref for timeouts: https://stackoverflow.com/questions/42873285/curl-retry-mechanism
// curl -o sun_rise_sun_set.json "https://api.sunrise-sunset.org/json?lat=42.1867052&lng=-86.2605776"
// const string CURL_SUN_RISE_SET_CMD = "curl -s -o " + SUN_RESPONSE_FILE
//                                    + " \"http://api.sunrise-sunset.org/json?lat=[lat]&lng=[lng]\"";

// use navy site for runrise/set times since is uses https instead of sunrise-sunset.org
// that uses only http (their documentation is not updates, still says https)
const string CURL_SUN_RISE_SET_CMD = "curl -s -o " + SUN_RESPONSE_FILE
             + " \"https://aa.usno.navy.mil/api/rstt/oneday?date=[date]&coords=[lat],[lng]\"";


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
https://api.sunrise-sunset.org/json?lat=42.1867052&lng=-86.2605776
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

https://aa.usno.navy.mil/api/rstt/oneday?date=[date]&coords=[lat],[lng]
{
  "apiversion": "3.0.0", 
  "geometry": {
    "coordinates": [
      -86.3, 
      42.2
    ], 
    "type": "Point"
  }, 
  "properties": {
    "data": {
      "closestphase": {
        "day": 28, 
        "month": 7, 
        "phase": "New Moon", 
        "time": "17:55", 
        "year": 2022
      }, 
      "curphase": "Waxing Crescent", 
      "day": 30, 
      "day_of_week": "Saturday", 
      "fracillum": "3%", 
      "isdst": false, 
      "label": null, 
      "month": 7, 
      "moondata": [
        {
          "phen": "Set", 
          "time": "02:10"
        }, 
        {
          "phen": "Rise", 
          "time": "12:12"
        }, 
        {
          "phen": "Upper Transit", 
          "time": "19:30"
        }
      ], 
      "sundata": [
        {
          "phen": "Set", 
          "time": "01:07"
        }, 
        {
          "phen": "End Civil Twilight", 
          "time": "01:39"
        }, 
        {
          "phen": "Begin Civil Twilight", 
          "time": "10:04"
        }, 
        {
          "phen": "Rise", 
          "time": "10:36"
        }, 
        {
          "phen": "Upper Transit", 
          "time": "17:52"
        }
      ], 
      "tz": 0.0, 
      "year": 2022
    }
  }, 
  "type": "Feature"
}

*/

