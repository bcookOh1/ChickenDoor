
#include "DateTimeUtils.h"

// tests the param hour againat the system real time clock in local time
/// param hour [1:23]
bool IsTime(int64_t hour, int64_t minute) {
   ptime now = second_clock::local_time();
   int64_t h = now.time_of_day().hours();
   int64_t m = now.time_of_day().minutes();
   return (h == hour && m == minute);
} // end IsHour


int ParseTime(const string &timeStr, std::tuple<unsigned, unsigned, unsigned> &val) {
   int ret = 0;

   unsigned hours = 0;
   unsigned minutes = 0;
   unsigned seconds = 0;

   smatch capture;
   if (regex_search(timeStr, capture, Sre_Time_String)) {

      hours = boost::lexical_cast<unsigned>(capture[1]);
      minutes = boost::lexical_cast<unsigned>(capture[2]);
      seconds = boost::lexical_cast<unsigned>(capture[3]);
      if (capture[4] == "PM") hours += 12;

      val = make_tuple(hours, minutes, seconds);
   }
   else {
      ret = -1;
   } // end if 

   return ret;
} // end ParseTime 


// parse a SunriseSunsetReader string into a tuple with <hour, min, sec>
int ParseDuration(const string& timeStr, std::tuple<unsigned, unsigned, unsigned>& val) {
   int ret = 0;

   unsigned hours = 0;
   unsigned minutes = 0;
   unsigned seconds = 0;

   smatch capture;
   if (regex_search(timeStr, capture, Sre_Duration_String)) {

      hours = boost::lexical_cast<unsigned>(capture[1]);
      minutes = boost::lexical_cast<unsigned>(capture[2]);
      seconds = boost::lexical_cast<unsigned>(capture[3]);
      val = make_tuple(hours, minutes, seconds);
   }
   else {
      ret = -1;
   } // end if 

   return ret;
} // end ParseDuration


int Parse24HrTime(const string& timeStr, std::tuple<unsigned, unsigned, unsigned>& val) {
   int ret = 0;

   unsigned hours = 0;
   unsigned minutes = 0;
   unsigned seconds = 0;

   smatch capture;
   if (regex_search(timeStr, capture, Sre_24Hr_Time_String)) {

      hours = boost::lexical_cast<unsigned>(capture[1]);
      minutes = boost::lexical_cast<unsigned>(capture[2]);

      if(capture.length(3) > 0){
         seconds = boost::lexical_cast<unsigned>(capture[3]);
      } // end if 

      val = make_tuple(hours, minutes, seconds);
   }
   else {
      ret = -1;
   } // end if 

   return ret;
} // end Parse24HrTime 


// make a ptime, use locate date and use hours, minutes, seconds,
// the time is in UTC
int MakePTimeFrom(unsigned hours, unsigned minutes, unsigned seconds, ptime &pt) {
   if (hours >= 24 || minutes >= 60 || seconds >= 60) return -1;

   ptime now = second_clock::local_time();
   pt = ptime{{now.date()},{hours, minutes, seconds}};

   return 0;
} // end MakePTimeFrom


int MakeDurationFrom(unsigned hours, unsigned minutes, unsigned seconds, time_duration& td) {
   td = time_duration{ hours, minutes, seconds };
   return 0;
} // end MakeDurationFrom


// just use the difference from utc and local time
// to convert utc times to locals time.
// ptime and time_duration handle the details on dates and 
// possible carries
int ToLocalTime(const ptime &utc_pt, ptime &local_pt) {

   ptime curr_time = second_clock::local_time();
   ptime utc_time = second_clock::universal_time();

   time_duration tz_offset = curr_time - utc_time;   
   local_pt = utc_pt + tz_offset;

   return 0;
} // end ToLocalTime


// another date formatter, this produces the right date string 
// for https://aa.usno.navy.mil (note their documentation is not up to date).
// this function returns the right format 
string GetNavyFormattedDate() {
   ostringstream oss;

   // get now time and convert to tm struct 
   time_t nowTime;
   time(&nowTime);
   struct tm* timeinfo;
   timeinfo = localtime(&nowTime);

   oss << setw(4) << (timeinfo->tm_year + 1900) << "-";
   oss << setw(2) << setfill('0') << (timeinfo->tm_mon + 1) << "-";
   oss << setw(2) << setfill('0') << timeinfo->tm_mday;

   return oss.str();;
} // end GetNavyFormattedDate


string Ptime2TmeString(const ptime &time){
   ostringstream oss;

   auto tod = time.time_of_day();
   oss << setw(2) << setfill('0') << tod.hours() << ":"; 
   oss << setw(2) << setfill('0') << tod.minutes() << ":"; 
   oss << setw(2) << setfill('0') << tod.seconds(); 

   return oss.str();
} // end Ptime2TmeString


// pre: sunrise contains the date for the sunrise (the date requested from the curl command)
// using https://api.sunrise-sunset.org/json or // used for https://aa.usno.navy.mil request 
// the date is date of request, MakePTimeFrom() adds the current date, assume MakePTimeFrom() 
// is called the day of the request
int CalcSunset(const ptime &sunrise, time_duration &dt, ptime &sunset) {
   sunset = sunrise + dt;
   return 0;
} // end CalcSunset


