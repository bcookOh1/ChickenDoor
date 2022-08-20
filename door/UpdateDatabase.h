// file: UpdateDatabase.h header for UpdateDatabase
// author: Bennett Cook
// date: 07-05-2020
// 

// header guard
#ifndef UPDATEDATABASE_H
#define UPDATEDATABASE_H

#include "sqlite3.h"


#include <string>
#include <tuple>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

// class to open and add a record to the database. The database is 
// how data is passes to the web page  
class UpdateDatabase {
public:

  UpdateDatabase();
  ~UpdateDatabase();

  int SetDbFullPath(const string &fullPath);
  int SetDoorStateTableName(const string &dbDoorStateTable);
  int SetSensorDataTableName(const string &dbSensorDataTable);
  int SetSunDataTableName(const string &dbSunDataTable);

  int OpenAndBeginDB();
  int CommitAndCloseDB();

  int AddDoorStateRow(const string &timestamp, 
                      int state,
                      const string &light, 
                      const string &temperature,
                      const string &decision);


  int AddOneDoorStateRow(const string &timestamp, 
                         int state, 
                         const string &light,
                         const string &temperature,
                         const string &decision);


  int AddSensorDataRow(const string &timeStamp, 
                       const string &temperature,
                       const string &temperature_units,
                       const string &humidity,
                       const string &humidity_units,
                       const string &light,
                       const string &light_units);


  int AddOneSensorDataRow(const string &timeStamp, 
                         const string &temperature,
                         const string &temperature_units,
                         const string &humidity,
                         const string &humidity_units,
                         const string &light,
                         const string &light_units);


  int AddOneSunDataRow(const string &timestamp, 
                       const string &sunrise,
                       const string &sunset);

  string GetErrorStr() { return _errorStr; }

private: 

  string _dbFullPath;
  string _dbDoorStateTable;
  string _dbSensorDataTable;
  string _dbSunDataTable;
  string _errorStr;
  sqlite3 *_db;

   // example from documentation
   static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
      int i;
      for(i = 0; i<argc; i++) {
         printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      }
      printf("\n");
      return 0;
   } // end error callback
 
}; // end class

#endif // end header guard