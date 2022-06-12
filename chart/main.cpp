#include "gnuplot.hpp"
#include "sqlite3.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;
using namespace boost;

// g++ -Wall -g -std=c++2a -ognup main.cpp gnuplot.hpp -lsqlite3
// see: https://stackoverflow.com/questions/31146713/sqlite3-exec-callback-function-clarification

string ToStdStr(const unsigned char *in){
   return string(reinterpret_cast<const char*>(in));
} // end ToStdStr


int main(int argc, char* argv[]){
   
   int ret = 0;
   char *zErrMsg = 0;
   string errorStr;
   const string dbFullPath("/home/bjc/coop/exe/coop.db");
   const string dbSensorDataTable("readings");
   const string gnuplotpDataFile("readings.txt");
   sqlite3 *db;
   sqlite3_stmt *stmt;

   float max = -100.0f; // for max temp in query
 
   // open db use full path 
   int rc = sqlite3_open(dbFullPath.c_str(), &db);
   if(rc != SQLITE_OK) {
      cout << "error: " << sqlite3_errmsg(db) << endl;
      sqlite3_free(zErrMsg);
      return -1;
   } // end if 
   
   // query string for last 24 hours 
   string sql = "select timestamp,temperature,humidity,light "
                "from " + dbSensorDataTable + " " +
                "where timestamp >= datetime((select max(timestamp) from readings),'-1 day') " +
                "order by id;";

   rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
   if(rc != SQLITE_OK) {
      cout << "error: " << sqlite3_errmsg(db) << endl;
      sqlite3_free(zErrMsg);
      ret = -1;
   }
   else {
   
      // open the file for writing, the file is name this 'readings.txt'
      // located here /var/www/html and set the owner and permissions like this:  
      // -rw-rw-rw- 1 www-data www-data 
      ofstream out (gnuplotpDataFile.c_str(), ios::out|ios::trunc);
      if (out.is_open()){

            // header for columns used in gnuplot 
            out << "timestamp,temperature (degF),humidity (%),light (lx)" << endl;

            // read and output each row 
            while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
               string timestamp = ToStdStr(sqlite3_column_text(stmt, 0));
               string temperature = ToStdStr(sqlite3_column_text(stmt, 1));
               string humidity = ToStdStr(sqlite3_column_text(stmt, 2));
               string light = ToStdStr(sqlite3_column_text(stmt, 3));
               out << timestamp << "," << temperature << "," << humidity << "," << light << endl;

               // convert and compare to get temp max in query
               // and ignore on error 
               try {
                  float value = lexical_cast<float>(temperature);
                  if(value > max) max = value;
               }
               catch(const bad_lexical_cast &){} 

            } // end while 

         out.close();
      }
      else {
         cout << "error: can't open data file for reading" << endl;
         ret = -1;
      } // end if 
   
   } // end if 

   if(rc != SQLITE_DONE) {
      cout << "error: " << sqlite3_errmsg(db) << endl;
      sqlite3_free(zErrMsg);
      ret = -1;
   } // end if

   sqlite3_finalize(stmt);
   sqlite3_close(db);

   // if no error make the plot 
   if(ret == 0) {

      // make a png file using gnuplot sending these commands 
      // through a linux pipe
      
      GnuPlot gp;
   /*
      // multi plot, with stacked plots
      gp("set terminal pngcairo size 800,400");
      gp("set output 'chart.png'");
      gp("set datafile separator ','");
      gp("set key autotitle columnhead");
      gp("set timefmt '%Y-%m-%d %H:%M:%S'");
      gp("set multiplot layout 2, 1 title 'Garage Temperature and Humidity' font ',14'");
      gp("set title 'Temperature in degF'");
      gp("set xdata time");
      gp("set format x \"%H:%M\"");
      gp("set key off");
      gp("set grid ytics lt 0 lw 1 lc rgb '#bbbbbb'");
      gp("set grid xtics lt 0 lw 1 lc rgb '#bbbbbb'");
      gp("plot [] [0:100] 'readings.txt' using 1:2 w l");
      gp("set title '% Relative Humidity'");
      gp("set xdata time");
      gp("set format x \"%H:%M\"");
      gp("set key off");
      gp("set grid ytics lt 0 lw 1 lc rgb '#bbbbbb'");
      gp("set grid xtics lt 0 lw 1 lc rgb '#bbbbbb'");
      gp("plot [] [0:100] 'readings.txt' using 1:3 w l");
   */
      // single plot with two traces
      gp("set terminal pngcairo size 800,400");
      gp("set output 'chart.png'");
      gp("set style line 1 lc 'dark-red' lw 2");
      gp("set style line 2 lc 'blue' lw 2");
      gp("set style line 3 lc 'yellow' lw 2");
      gp("set title 'Chicken Coop Temperature, Humidity, Light' font ',12'");
      gp("set border 3 lw 2");
      gp("set key autotitle columnhead");
      gp("set key bottom right");
      gp("set key font ',10'");
      gp("set grid");
      gp("set tics font ',10'");
      gp("set tics nomirror");
      gp("set ylabel 'degF or %' font ',10'");
      gp("set xlabel 'hour:minute' font ',10'");
      gp("set xlabel offset -1");
      gp("set datafile separator ','");
      gp("set xdata time");
      gp("set timefmt '%Y-%m-%d %H:%M:%S'");
      gp("set format x '%H:%M'");
      gp("set ytics 0,10,100");
      gp("set xtics rotate by 45 offset -2.3,-1.2");
      string plotString = "plot [] [0:placeholder] 'readings.txt' using 1:2 w l ls 1,'readings.txt' using 1:3 w l ls 2,'readings.txt' using 1:4 w l ls 3";

      // adjust the y max range up if needed 
      string rangeMax = "100";
      if(max >= 100.0f){
         
         // ignore error and use default rangeMax
         try {
            rangeMax = lexical_cast<string>(static_cast<int>(max + 5));
         }
         catch(const bad_lexical_cast &){} 

      } // end if 

      replace_first(plotString, "placeholder", rangeMax);

      gp(plotString.c_str());

      } // end if 

   return ret;
} // end main

