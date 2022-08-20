
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <tuple>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost::xpressive;
using namespace boost::posix_time;
using namespace boost::gregorian;


struct SunTimes {
   ptime rise;
   ptime set;
   void Print() { cout << "sunrise: " << rise << ", sunset: " << set << endl; }
}; // end struct

// will not work for "midnight sun" locations, this is if latitude >= 66.5deg north

// time string from SunriseSunsetReader, ex "10:32:47 AM"
const sregex Sre_Time_String = (s1 = +_d) >> ':' >> (s2 = +_d) >> ':' >>
                               (s3 = +_d) >> +_s >> (s4 = +upper);

// time string from SunriseSunsetReader, ex "15:19:00"
const sregex Sre_Duration_String = (s1 = +_d) >> ':' >> (s2 = +_d) >> ':' >>
                                   (s3 = +_d);

// 24hr time string from configuration ex "02:10:00", note seconds are optional
const sregex Sre_24Hr_Time_String = (s1 = +_d) >> ':' >> (s2 = +_d) >> 
   boost::xpressive::optional(':' >> (s3 = +_d));

bool IsTime(int64_t hour, int64_t minute);

// parse a SunriseSunsetReader string into a tuple with <hour, min, sec>
// example: cout << "hour: " << get<0>(val) << ", minute: " << get<1>(val) << ", second: " << get<2>(val);
int ParseTime(const string &timeStr, std::tuple<unsigned, unsigned, unsigned> &val);
int ParseDuration(const string& timeStr, std::tuple<unsigned, unsigned, unsigned>& val);
int Parse24HrTime(const string& timeStr, std::tuple<unsigned, unsigned, unsigned>& val);

// ptime is in  are in UTC
int MakePTimeFrom(unsigned hours, unsigned minutes, unsigned second, ptime &pt);
int MakeDurationFrom(unsigned hours, unsigned minutes, unsigned second, time_duration &td);

int ToLocalTime(const ptime &utc_pt, ptime &local_pt);
int CalcSunset(const ptime &pt, time_duration &dt, ptime &sunset);

// used for https://aa.usno.navy.mil request 
string GetNavyFormattedDate();

string Ptime2TmeString(const ptime &time);


// all times are in local times 
// the offsets are set in the ctor so the class is valid for those initial values only,
// sunriseOffset and sunsetOffset are read from the configurtion file 
// but the sunrise sunset times are expected to be updated each day
class Daytime {
public:
   // params sunriseOffset and sunsetOffset are in minutes
   Daytime(int sunriseOffset, int sunsetOffset) : _daytime{0} {
      _sunriseOffset = minutes{ sunriseOffset }; 
      _sunsetOffset = minutes{ sunsetOffset };
   } // end ctor 

   ~Daytime() {}

   // the sunrise and sunset values are update each day 
   // pre: sunrise and sunset must include the date 
   void SetSunriseSunsetTimes(const ptime& sunrise, const ptime& sunset) {
       _sunrise = sunrise; _sunset = sunset; 
   } // end SetSunriseSunsetTimes
   
   // return true is day time or false not day time    
   // note: the sunset always follows sunrise, ptime variables include the date
   // so using ptimes the comparsion is always valid
   int IsDaytime() { 
      ptime sunriseCompare = _sunrise + _sunriseOffset;
      ptime sunsetCompare = _sunset + _sunsetOffset;
      ptime now = second_clock::local_time();

      if (now >= sunriseCompare && now < sunsetCompare) {
         _daytime = 1;
      } // end if 

      if (now >= sunsetCompare) {
         _daytime = 0;
      } // end if 

      return _daytime;
   } // end IsDaytime

private:
   int _daytime;
   ptime _sunrise;
   ptime _sunset;
   time_duration _sunriseOffset;
   time_duration _sunsetOffset;
}; // end class

