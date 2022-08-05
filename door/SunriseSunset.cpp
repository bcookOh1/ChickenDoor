#include "SunriseSunset.h"

SunriseSunset::SunriseSunset(const Address &address, const string &updateAt) {
   Setup(address, updateAt);
   _status = SunriseSunsetStatus::Initial;
   _hasCoords = false;
} // end ctor 

SunriseSunset::~SunriseSunset() {}

int SunriseSunset::Setup(const Address &address, const string &updateAt) {

   _address.house_number = address.house_number;
   _address.street = address.street;
   _address.city = address.city;
   _address.state = address.state;
   _address.zip_code = address.zip_code;

   std::tuple<unsigned, unsigned, unsigned> val;
   int ret = Parse24HrTime(updateAt, val);
   if (ret == 0) {
      _fetchHour = get<0>(val);
      _fetchMinute = get<1>(val);
   }
   else { // set defaults
      _fetchHour = 2;
      _fetchHour = 5;
   } // end if 

   return ret;
} // end Setup


void SunriseSunset::FetchTimes(coroutine<SunriseSunsetStatus>::push_type& out) {
   _status = SunriseSunsetStatus::Initial;
   _hasCoords = false;

   LocationReader lr(_address);
   SunriseSunsetReader sr;

   // continuous loop, coroutine life time controlled by caller,
   // all "out(_status);" are the "co_await" like returns
   while(true) {

      // always run on first loop, if successfull just reuse data 
      // and skip in all later loops
      if (_hasCoords == false) {

         lr.ReadAfterSec(2);
         _status = SunriseSunsetStatus::CoordsStarted;
         out(_status);

         while (lr.GetStatus() == ReaderStatus::Waiting) {
            out(_status);
         } // end while

         if (lr.GetStatus() == ReaderStatus::Complete) {
            _status = SunriseSunsetStatus::CoordsComplete;
            _hasCoords = true;
         }
         else if (lr.GetStatus() == ReaderStatus::Error) {
            _status = SunriseSunsetStatus::Error;
            _errorStr = lr.GetError();
         } // end if

      }
      else {
         _status = SunriseSunsetStatus::CoordsComplete;
      } // end if 

      out(_status);

      // use default times on LocationReader fail
      if (_status == SunriseSunsetStatus::CoordsComplete) {

         Coordinates data = lr.GetData();
         sr.SetCoords(data);
         sr.ReadAfterSec(2);
         _status = SunriseSunsetStatus::SunRiseSetStarted;

         while (sr.GetStatus() == ReaderStatus::Waiting) {
            out(_status);
         } // end while 

         if (sr.GetStatus() == ReaderStatus::Complete) {
            _status = SunriseSunsetStatus::SunRiseSetComplete;
            _times = sr.GetData();
            sr.ResetStatus();

            _times.Print();
            
            // this conversion is multi-step with several possible errors 
            int result = Convert2Local();
            if (result != 0) {
               _status = SunriseSunsetStatus::Error;
            } // end if 

         }
         else if (sr.GetStatus() == ReaderStatus::Error) {
            _status = SunriseSunsetStatus::Error;
            _errorStr = sr.GetError();
         } // end if 

         out(_status);

      } // end if
      
      _status = SunriseSunsetStatus::WaitForNextDay;

      bool onetime = false;

      // wait here until IsTime() is true 
      while(_status == SunriseSunsetStatus::WaitForNextDay) {

         if (IsTime(_fetchHour, _fetchMinute) == true) 
            onetime = true;

         if(onetime == true && IsTime(_fetchHour, _fetchMinute) != true)
            _status = SunriseSunsetStatus::Initial;

         out(_status);
      } // end while

   } // end while

   return;
} // end FetchTimes


int SunriseSunset::Convert2Local() {
   int result = 0;

   std::tuple<unsigned, unsigned, unsigned> val1, val2;
   ptime utc_sunrise_pt, utc_sunset_pt;
   time_duration day_length;

   // for sunsise
   result = ParseTime(_times.sunrise, val1);
   if (result != 0) {
      _errorStr = "error parsing sunrise string";
      return -1;
   } // end if 

   // cout << "hour: " << get<0>(val1) << ", minute: " << get<1>(val1) << ", second: " << get<2>(val1) << endl;

   // for duration
   result = ParseDuration(_times.day_length, val2);
   if (result != 0) {
      _errorStr = "error parsing day_length string";
      return -1;
   } // end if 

   // cout << "hour: " << get<0>(val2) << ", minute: " << get<1>(val2) << ", second: " << get<2>(val2) << endl;

   result = MakePTimeFrom(get<0>(val1), get<1>(val1), get<2>(val1), utc_sunrise_pt);
   if (result != 0) {
      _errorStr = "error on sunrise values to ptime";
      return -1;
   } // end if 

   result = MakeDurationFrom(get<0>(val2), get<1>(val2), get<2>(val2), day_length);
   if (result != 0) {
      _errorStr = "error on day_length values to time_duration";
      return -1;
   } // end if 

   result = CalcSunset(utc_sunrise_pt, day_length, utc_sunset_pt);
   if (result != 0) {
      _errorStr = "error on calculating sunset";
      return -1;
   } // end if 

   result = ToLocalTime(utc_sunrise_pt, _localTimes.rise);
   if (result != 0) {
      _errorStr = "error on convert sunrise to local time";
      return -1;
   } // end if 

   result = ToLocalTime(utc_sunset_pt, _localTimes.set);
   if (result != 0) {
      _errorStr = "error on convert sunset to local time";
      return -1;
   } // end if 

   return 0;
} // end Convert2Local


