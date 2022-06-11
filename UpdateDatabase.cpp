
#include "UpdateDatabase.h"


UpdateDatabase::UpdateDatabase(){
   _errorStr = "success";
   _db = nullptr;
} // end ctor 


UpdateDatabase::~UpdateDatabase() {
} // end dtor 


int UpdateDatabase::SetDbFullPath(const string &fullPath) {
    int ret = 0;
    _dbFullPath = fullPath;
    return ret;
  } // end SetDbFilename


int UpdateDatabase::SetDoorStateTableName(const string &dbDoorStateTable) {
   int ret = 0;
   _dbDoorStateTable = dbDoorStateTable;
   return ret;
} // end SetDoorStateTableName


int UpdateDatabase::SetSensorDataTableName(const string &dbSensorDataTable){
   int ret = 0;
   _dbSensorDataTable = dbSensorDataTable;
   return ret;
} // end SetSensorDataTableName


int UpdateDatabase::OpenAndBeginDB(){
   int ret = 0;
   char *zErrMsg = 0;

    // open db use full path 
   int rc = sqlite3_open(_dbFullPath.c_str(), &_db);
   if(!rc) {

      // execute sql statement 
      rc = sqlite3_exec(_db, "begin", callback, 0, &zErrMsg);
      if( rc != SQLITE_OK ){
         _errorStr = "begin command error: ";
         _errorStr += sqlite3_errmsg(_db);
         sqlite3_free(zErrMsg);
         sqlite3_close(_db);
         ret = -1;
      } // end if

   }
   else {
      _errorStr = "can't open database: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      ret = -1;
   } // end if 

  
   return ret;
} // end OpenAndBegin


int UpdateDatabase::CommitAndCloseDB(){
   int ret = 0;
   char *zErrMsg = 0;

     // execute end (commit) transition 
   int rc = sqlite3_exec(_db, "end", callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "begin command error: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      ret = -1;
   } // end if 
   
   sqlite3_close(_db);
   _db = nullptr;
 
   return ret;
} // end CommitAndCloseDB


int UpdateDatabase::AddDoorStateRow(const string &rec_time, 
                                    int state, 
                                    const string &temperature) {
   int ret = 0;
   char *zErrMsg = 0;

   // build a insert sql command 
   string sql = "insert into" + _dbDoorStateTable + "(rec_time, state, temperature) values ";
   sql += "('" + rec_time  + "', " + lexical_cast<string>(state) + ", '" + temperature + "')";

   // execute sql statement to insert a row
   int rc = sqlite3_exec(_db, sql.c_str(), callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "insert row: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      ret = -1;
   } // end if 

   return ret;
} // end AddDoorStateRow


int UpdateDatabase::AddOneDoorStateRow(const string &rec_time, 
                                       int state, 
                                       const string &temperature) {
   int ret = 0;
   char *zErrMsg = 0;

   // open db use full path 
   int rc = sqlite3_open(_dbFullPath.c_str(), &_db);
   if(rc) {
      _errorStr = "can't open database: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      return -1;
   } // end if 
 

   // execute sql statement 
   rc = sqlite3_exec(_db, "begin", callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "begin command error: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      return -1;
   } // end if 


   // build a insert sql command 
   string sql = "insert into " + _dbDoorStateTable + "(rec_time, state, temperature) values ";
   sql += "('" + rec_time  + "', " + lexical_cast<string>(state) + ", '" + temperature + "')";

   // execute sql statement to insert a row
   rc = sqlite3_exec(_db, sql.c_str(), callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "insert row: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      return -1;
   } // end if 


   // execute end (commit) transition 
   rc = sqlite3_exec(_db, "end", callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "begin command error: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      return -1;
   } // end if 
   
   sqlite3_close(_db);

   return ret;
} // end AddOneDoorStateRow


int UpdateDatabase::AddSensorDataRow(const string &timestamp, 
                                     const string &temperature,
                                     const string &temperature_units,
                                     const string &humidity,
                                     const string &humidity_units){
   int ret = 0; 
   char *zErrMsg = 0;

   // build a insert sql command 
   string sql = "insert into " + _dbSensorDataTable + "(timestamp, temperature, " +
                "temperature_units, humidity, humidity_units) values";
   sql += "('" + timestamp  + "', '" + temperature + "', '" + temperature_units + "', '"
               + humidity  + "', '" + humidity_units + "')";

   // execute sql statement to insert a row
   int rc = sqlite3_exec(_db, sql.c_str(), callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "insert row: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      ret = -1;
   } // end if 

   return ret;
} // end AddSensorDataRow


int UpdateDatabase::AddOneSensorDataRow(const string &timestamp, 
                                        const string &temperature,
                                        const string &temperature_units,
                                        const string &humidity,
                                        const string &humidity_units){
   int ret = 0; 
   char *zErrMsg = 0;

   // open db use full path 
   int rc = sqlite3_open(_dbFullPath.c_str(), &_db);
   if(rc) {
      _errorStr = "can't open database: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      return -1;
   } // end if 
 

   // execute sql statement 
   rc = sqlite3_exec(_db, "begin", callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "begin command error: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      return -1;
   } // end if 


   // build a insert sql command 
   string sql = "insert into " + _dbSensorDataTable + "(timestamp, temperature, " +
                "temperature_units, humidity, humidity_units) values";
   sql += "('" + timestamp  + "', '" + temperature + "', '" + temperature_units + "', '"
               + humidity  + "', '" + humidity_units + "')";

   // execute sql statement to insert a row
   rc = sqlite3_exec(_db, sql.c_str(), callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "insert row: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      return -1;
   } // end if 


   // execute end (commit) transition 
   rc = sqlite3_exec(_db, "end", callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      _errorStr = "begin command error: ";
      _errorStr += sqlite3_errmsg(_db);
      sqlite3_free(zErrMsg);
      sqlite3_close(_db);
      return -1;
   } // end if 
   
   sqlite3_close(_db);

   return ret;
} // end AddOneSensorDataRow




