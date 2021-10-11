
#include "Util.h"


string GetSqlite3DateTime() {

   string ret;
   ostringstream oss;

   // get now time and convert to tm struct 
   time_t nowTime;
   time(&nowTime);
   struct tm *timeinfo;
   timeinfo = localtime(&nowTime);

   oss << setw(4) << (timeinfo->tm_year + 1900) << "-";
   oss << setw(2) << setfill('0') << (timeinfo->tm_mon + 1) << "-";
   oss << setw(2) << setfill('0') << timeinfo->tm_mday << " ";
   oss << setw(2) << setfill('0') << timeinfo->tm_hour << ":";
   oss << setw(2) << setfill('0') << timeinfo->tm_min << ":";
   oss << setw(2) << setfill('0') << timeinfo->tm_sec;
   ret = oss.str();

    return ret;
} // end GetSqlite3DateTime


string GetDateTimeFilename() {

   string ret;
   ostringstream oss;

   // get now time and convert to tm struct 
   time_t nowTime;
   time(&nowTime);
   struct tm *timeinfo;
   timeinfo = localtime(&nowTime);

   oss << setw(4) << (timeinfo->tm_year + 1900) << "-";
   oss << setw(2) << setfill('0') << (timeinfo->tm_mon + 1) << "-";
   oss << setw(2) << setfill('0') << timeinfo->tm_mday << "_";
   oss << setw(2) << setfill('0') << timeinfo->tm_hour;
   oss << setw(2) << setfill('0') << timeinfo->tm_min;
   oss << setw(2) << setfill('0') << timeinfo->tm_sec;
   ret = oss.str();

    return ret;
} // end GetDateTimeFilename



IoValues MakeIoValuesMap(const vector<IoConfig> &io) {
   IoValues ret;

   for(auto iter = io.begin(); iter != io.end(); ++iter) {
      ret.insert(make_pair(iter->name, 0));
   } // end for 

   return ret;  
} // end MakeIoValuesMap


void PrintIo(const IoValues &ioValues){

   ostringstream oss;

   // title and column header
   oss << "\nIO Values\n";
   oss << "name,value\n";

   // print data 
   for(auto iter = ioValues.begin(); iter != ioValues.end(); ++iter) {
      oss << iter->first << ", " << iter->second << "\n";
   } // end for 

   string out = oss.str();
   cout << out;

} // end PrintIo

string IoToString(const IoValues &ioValues){

   ostringstream oss;

   // title and column header
   oss << "\nIO Values\n";
   oss << "name,value\n";

   // print data 
   for(auto iter = ioValues.begin(); iter != ioValues.end(); ++iter) {
      oss << iter->first << ", " << iter->second << "\n";
   } // end for 

   return oss.str();
} // end IoToString

string IoToLine(const IoValues &ioValues){

   ostringstream oss;

   // print data 
   for(auto iter = ioValues.begin(); iter != ioValues.end(); ++iter) {
      oss << iter->first << ": " << iter->second << ", ";
   } // end for 
   
   string out = oss.str();
   out.pop_back();
   out += "\n";

   return out;
} // end IoToString


/*
int ReadBoardTemperature(string &temperature){
   temperature = "";
   string filepath = "/sys/class/thermal/thermal_zone0/temp";
   ifstream in; 

   in.open(filepath.c_str()); // open the file
   if(in.fail()) {
      return -1;
   } // end if

   temperature = "";
   getline(in, temperature);
   in.close();

   // insert a period char 3 from the end 
   try {
      temperature.insert((temperature.end() - 3),'.');
   }
   catch(length_error *le) {
      temperature = "";
      return -1;
   }  // end catch 

   return 0; // still here all good
} // end ReadBoardTemperature 


void UpdateDoorStateDB(DoorState ds, UpdateDatabase &udb, string temperature) {
   int result = udb.AddOneDoorStateRow(GetSqlite3DateTime(), static_cast<int>(ds), temperature);
   if(result != 0){
      cout << "database write error: " << udb.GetErrorStr() << endl; 
   } // end if 
   return;
} // end UpdateDoorStateDB

*/



 