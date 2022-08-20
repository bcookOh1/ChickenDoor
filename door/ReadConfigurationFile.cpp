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
      _appConfig.dbSunDataTable = GetScalarData<string>(tree, CONFIG_DB_SUN_DATA_TABLE);

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
      _appConfig.pwmHzFast = GetScalarData<int>(tree, CONFIG_FAST_PWM_HZ);
      _appConfig.pwmHzSlow = GetScalarData<int>(tree, CONFIG_SLOW_PWM_HZ);
      _appConfig.pwmHzHoming = GetScalarData<int>(tree, CONFIG_HOMING_PWM_HZ);
      _appConfig.morningLight = GetScalarData<float>(tree, CONFIG_MORNING_LIGHT_LEVEL);
      _appConfig.nightLight = GetScalarData<float>(tree, CONFIG_NIGHT_LIGHT_LEVEL);
      _appConfig.sensorReadIntervalSec = GetScalarData<int>(tree, CONFIG_SENSOR_READ_INTERVAL_SEC);
      _appConfig.sunriseOffsetMin = GetScalarData<int>(tree, CONFIG_SUNRISE_OFFSET_MINUTES);
      _appConfig.sunsetOffsetMin = GetScalarData<int>(tree, CONFIG_SUNSET_OFFSET_MINUTES);

      _appConfig.houseNumber = GetScalarData<string>(tree, CONFIG_ADDRESS_HOUSE_NUMBER);
      _appConfig.street = GetScalarData<string>(tree, CONFIG_ADDRESS_STREET);
      _appConfig.city = GetScalarData<string>(tree, CONFIG_ADDRESS_CITY);
      _appConfig.state = GetScalarData<string>(tree, CONFIG_ADDRESS_STATE);
      _appConfig.zipCode = GetScalarData<string>(tree, CONFIG_ADDRESS_ZIP_CODE);

   }
   catch(std::exception &e) {
      _errorStr = "error on read ";
      _errorStr += e.what();
      ret = -1;
   } // end try/catch

   return ret;
} // end ReadIn

