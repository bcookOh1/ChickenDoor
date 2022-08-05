#pragma once

#include <string>
#include <chrono>
#include <boost/coroutine2/all.hpp>

#include "CommonDef.h"
#include "DateTimeUtils.h"
#include "LocationReader.h"
#include "SunriseSunsetReader.h"


using namespace std;
using namespace std::chrono;
using namespace boost::coroutines2;


enum class SunriseSunsetStatus : int {
   Initial = 0,
   CoordsStarted,
   CoordsComplete,
   SunRiseSetStarted,
   SunRiseSetComplete,
   WaitForNextDay,
   Error,
};

const int MAX_RETRIES = 3;


class SunriseSunset {
public:
   SunriseSunset(const Address &address, const string &updateAt);
   ~SunriseSunset();

   SunriseSunsetStatus GetStatus() { return _status; }
   SunTimes GetTimes() { return _localTimes; }
   string GetError() { return _errorStr; }

   void FetchTimes(coroutine<SunriseSunsetStatus>::push_type& out);

private:
   Address _address;
   Coordinates _coords;
   SunriseSunsetStringTimes _times;
   SunriseSunsetStatus _status;
   string _errorStr;
   bool _hasCoords;
   unsigned _fetchHour;
   unsigned _fetchMinute;
   SunTimes _localTimes;

   int Setup(const Address& address, const string& time);
   int Convert2Local();

}; // end class
