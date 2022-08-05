/// file: ReadConfigurationFile.h header for ReadConfigurationFile class  
/// author: Bennett Cook
/// date: 07-08-2020
/// description: read in a json file and and populate an config 
/// struct. This class uses a boost::property_tree as the read-in/container 
/// for the config. But the configuration data has to be converted to 
/// the AppConfig members so there are 3 template functions for 
/// the more complex conversions.


// header guard
#ifndef READCONFIGURATIONFILE_H
#define READCONFIGURATIONFILE_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

#include "CommonDef.h"
#include "Util.h"


using namespace std;
using namespace boost;
namespace pt = boost::property_tree;

/// \class 
/// \brief class to read the json file 
/// and parse the data string  
class ReadConfigurationFile {
public:

   /// \brief constructor
   ReadConfigurationFile();

   /// \brief destructor
   ~ReadConfigurationFile();

   /// \brief set and get for the configuration file name 
   void SetConfigFilename(string fname) { _configFilename = fname; }
   string GetConfigFilename() { return _configFilename; }

   /// \brief perform the json file read-in and convert element to AppConfig members.
   /// \return 0 success
   /// \return -1 an error occurred and the error string was set
   int ReadIn();

   /// \brief get function for the config 
   AppConfig GetConfiguration() { return _appConfig; }

   /// \brief get status string  
   string GetErrorStr() { return _errorStr; }

private:

   string _configFilename;      // a file name  
   string _errorStr;            // error string

   AppConfig _appConfig;        // the application data from the json file 


   /// \brief template function to read a 2d array from the property tree
   template<typename T>
   vector<vector<T>> Get2dData(pt::ptree tree, const string &child_label) {
      vector<vector<T>> ret;
      _errorStr = "";

      try {
         for(pt::ptree::value_type &v : tree.get_child(child_label.c_str())) {
            vector<T> temp;
            for(pt::ptree::value_type &v2 : v.second) {
               try {
                  temp.push_back(lexical_cast<T>(v2.second.data()));
               }
               catch(bad_lexical_cast &e) {
                  _errorStr = "error on lexical_cast ";
                  _errorStr += e.what();
               } // end try catch

               // if exception occurred throw  
               if(_errorStr.length() > 0) throw _errorStr.c_str();

            } // end for 
            ret.push_back(temp);
         } // end for 
      }
      catch(std::exception &e) {
         _errorStr = "error on child read ";
         _errorStr += e.what();
      } // end try/catch

      // if exception occurred throw  
      if(_errorStr.length() > 0) throw _errorStr.c_str();

      return ret;
   } // end Get2dData

   /// \brief template function to read a 1d array from the property tree
   template<typename T>
   vector<T> Get1dData(pt::ptree tree, const string &child_label) {
      vector<T> ret;
      _errorStr = "";

      try {
         for(pt::ptree::value_type &v : tree.get_child(child_label.c_str())) {
            try {
               ret.push_back(lexical_cast<T>(v.second.data()));
            }
            catch(bad_lexical_cast &e) {
               _errorStr = "error on lexical_cast ";
               _errorStr += e.what();
            } // end try catch

            // if exception occurred throw  
            if(_errorStr.length() > 0) throw _errorStr.c_str();
         } // end for 

      }
      catch(std::exception &e) {
         _errorStr = "error on child read ";
         _errorStr += e.what();
      } // end try/catch

      // if exception occurred throw  
      if(_errorStr.length() > 0) throw _errorStr.c_str();

      return ret;
   } // end Get1dData

   /// \brief template function to read a scalar from the property tree
   template<typename T>
   T GetScalarData(pt::ptree tree, const string &child_label) {
      T ret;
      _errorStr = "";

      try {
         ret = tree.get<T>(child_label);
      }
      catch(std::exception &e) {
         _errorStr = "error on child read ";
         _errorStr += e.what();
      } // end try/catch

      // if exception occurred throw  
      if(_errorStr.length() > 0) throw _errorStr.c_str();

      return ret;
   } // end GetScalarData

}; // end class 

#endif  // end header guard

