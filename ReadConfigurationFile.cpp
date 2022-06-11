/// file: ReadConfigurationFile.cpp Implementation for ReadConfigurationFile class  
/// author: Bennett Cook
/// date: 07-08-2020
/// update: 05-27-2021: add help light on time
/// description: 

#include "ReadConfigurationFile.h"

/// \brief constructor
ReadConfigurationFile::ReadConfigurationFile() {
} // end ctor 

/// \brief destructor
ReadConfigurationFile::~ReadConfigurationFile() {
} // end dtor 

/// \brief perform the json file read-in and convert element to 
/// AppConfig members.
int ReadConfigurationFile::ReadIn() {
   int ret = 0;

   pt::ptree tree;

   try {
      read_json(_configFilename, tree);

      _appConfig.appName = GetScalarData<string>(tree, CONFIG_APP_NAME);
      _appConfig.dbPath = GetScalarData<string>(tree, CONFIG_DB_PATH);
      _appConfig.dbDoorStateTable = GetScalarData<string>(tree, CONFIG_DB_DOOR_STATE_TABLE);
      _appConfig.dbSensorTable = GetScalarData<string>(tree, CONFIG_DB_SENSOR_TABLE);

      // get the IO configuration
      for(pt::ptree::value_type &v : tree.get_child(CONFIG_DIGITAL_IO)) {

         // get the individual elements of the IO, use try/catch to insure  
         // sure the types are correct
         try {

            IoConfig ioconfig;
            ioconfig.SetTypeFromString(v.second.get<string>(CONFIG_DIGITAL_IO_TYPE));
            ioconfig.name = v.second.get<string>(CONFIG_DIGITAL_IO_NAME);
            ioconfig.pin = v.second.get<unsigned>(CONFIG_DIGITAL_IO_PIN);

            // test if CONFIG_DIGITAL_INPUT_RESISTOR_MODE is in the tree
            // if found set ioconfig.resistor_mode else use None (default) 
            boost::optional<string> tmp = v.second.get_optional<string>(CONFIG_DIGITAL_INPUT_RESISTOR_MODE);
            if(tmp.is_initialized()) {
               ioconfig.SetInputResistorModeFromString(v.second.get<string>(CONFIG_DIGITAL_INPUT_RESISTOR_MODE));
            }
            else {
               ioconfig.resistor_mode =InputResistorMode::None;
            } // end if 

            // add to app config struct 
            _appConfig.dIos.push_back(ioconfig);
         }
         catch(std::exception &e) {
            _errorStr = "digital IO error on lexical_cast, ";
            _errorStr += e.what();
            return -1;
         } // end try catch

      } // end for 

      _appConfig.loopTimeMS = GetScalarData<int>(tree, CONFIG_LOOP_TIME_MS);
      _appConfig.fastPwmHz = GetScalarData<int>(tree, CONFIG_FAST_PWM_HZ);
      _appConfig.slowPwmHz = GetScalarData<int>(tree, CONFIG_SLOW_PWM_HZ);
      _appConfig.homingPwmHz = GetScalarData<int>(tree, CONFIG_HOMING_PWM_HZ);
      _appConfig.piTempReadIntervalSec = GetScalarData<int>(tree, CONFIG_PI_TEMP_READ_INTERVAL_SEC);

   }
   catch(std::exception &e) {
      _errorStr = "error on read ";
      _errorStr += e.what();
      ret = -1;
   } // end try/catch

   return ret;
} // end ReadIn

