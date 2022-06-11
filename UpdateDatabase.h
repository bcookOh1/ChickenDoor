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

  int OpenAndBeginDB();
  int CommitAndCloseDB();

  int AddDoorStateRow(const string &rec_time, 
                      int state, 
                      const string &temperature);

  int AddOneDoorStateRow(const string &rec_time, 
                         int state, 
                         const string &temperature);

  int AddSensorDataRow(const string &timeStamp, 
                       const string &temperature,
                       const string &temperature_units,
                       const string &humidity,
                       const string &humidity_units);

  int AddOneSensorDataRow(const string &timeStamp, 
                         const string &temperature,
                         const string &temperature_units,
                         const string &humidity,
                         const string &humidity_units);

  string GetErrorStr() { return _errorStr; }

private: 

  string _dbFullPath;
  string _dbDoorStateTable;
  string _dbSensorDataTable;
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