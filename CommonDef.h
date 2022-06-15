/// file: CommonDef.h
/// author: Bennett Cook
/// date: 07-05-2020
/// description: a place to put common program definitions. 
/// update: 05-27-2021: add help light on time constants


// header guard
#ifndef COMMONDEF_H
#define COMMONDEF_H

#include <string>
#include <string_view>
#include <map>
#include <thread>
#include <chrono>


using namespace std;

// outputs 
const int STEPPER_ENABLE = 29; 
const int STEPPER_DIRECTION = 25; 

// inputs 
const int DOOR_OBSTRUCTED = 28; 
const int DOOR_RETRACTED = 27; 
const int DOOR_EXTENDED = 26; 

// define an enum for the gpio type
// values match macros in wiringPi.h
enum class PinType {
   DInput = 0,
   DOutput
}; 

// define an enum for the digital input resistor
// values match macros in wiringPi.h
enum class InputResistorMode {
   None = 0,
   PullDown,
   PullUp
}; 

enum class DoorState : int {
   Startup = 0,
   Open,
   MovingToClose,
   Closed,
   MovingToOpen,
   Obstructed,
   NoChange
}; // end enum 

// alias for setting or reading io values 
//                  name,    value  
using IoValues = std::map<std::string, unsigned>;

// configuration file data names
const string CONFIG_APP_NAME = "ChickenCoop.name";
const string CONFIG_DB_PATH = "ChickenCoop.database_path";
const string CONFIG_DB_DOOR_STATE_TABLE = "ChickenCoop.door_state_table";
const string CONFIG_DB_SENSOR_TABLE = "ChickenCoop.sensor_table";
const string CONFIG_DIGITAL_IO = "ChickenCoop.digital_io";
const string CONFIG_DIGITAL_IO_TYPE = "type";
const string CONFIG_DIGITAL_IO_NAME = "name";
const string CONFIG_DIGITAL_INPUT_RESISTOR_MODE = "resistor_mode";
const string CONFIG_DIGITAL_IO_PIN = "pin";
const string CONFIG_LOOP_TIME_MS = "ChickenCoop.loop_time_ms";
const string CONFIG_FAST_PWM_HZ = "ChickenCoop.fast_pwm_hz";
const string CONFIG_SLOW_PWM_HZ = "ChickenCoop.slow_pwm_hz";
const string CONFIG_HOMING_PWM_HZ = "ChickenCoop.homing_pwm_hz";
const string CONFIG_SENSOR_READ_INTERVAL_SEC = "ChickenCoop.sensor_read_interval_sec";


// string values for digital io type 
const string DIGITAL_INPUT_STR = "input";
const string DIGITAL_OUTPUT_STR = "output";

// string values for digital input resistor mode 
const string INPUT_RESISTOR_NONE_STR = "none";
const string INPUT_RESISTOR_PULLDOWN_STR = "pulldown";
const string INPUT_RESISTOR_PULLUP_STR = "pullup";


// define a copyable struct for gpio configurations 
struct IoConfig {

   IoConfig() {
      type = PinType::DInput,
      pin = 0;
      resistor_mode = InputResistorMode::None;
   } // end ctor

   // copy constructor
   IoConfig(const IoConfig &rhs) {
      type = rhs.type;
      name = rhs.name;
      pin = rhs.pin;
      resistor_mode = rhs.resistor_mode; 
   } // end ctor

   // assignment operator
   IoConfig &operator=(const IoConfig &rhs) {
      type = rhs.type;
      name = rhs.name;
      pin = rhs.pin;
      resistor_mode = rhs.resistor_mode;
      return *this;
   } // end assignment operator

   // set type from string 
   void SetTypeFromString(string_view strv) {
      if(strv == DIGITAL_OUTPUT_STR){
         type = PinType::DOutput;
      }
      else {
         type = PinType::DInput;
      } // end if 
   } // end SetTypeFromString 

   // set mode from string 
   void SetInputResistorModeFromString(string_view strv) {
      if(strv == INPUT_RESISTOR_PULLDOWN_STR){
         resistor_mode = InputResistorMode::PullDown;
      }
      else if(strv == INPUT_RESISTOR_PULLUP_STR){
         resistor_mode = InputResistorMode::PullUp;
      }
      else {
         resistor_mode = InputResistorMode::None;
      } // end if 
   } // end SetTypeFromString 

   PinType type;
   string name; 
   unsigned pin;
   InputResistorMode resistor_mode;
}; // end struct

// simple struct with application configuration.
// the struct is copyable with copy constructor
// and assignment operator 
struct AppConfig  {

   // default constructor
   AppConfig() { 
      loopTimeMS = 0; 
      fastPwmHz = 0; 
      slowPwmHz = 0;
      homingPwmHz = 0;
      sensorReadIntervalSec = 30;
   } // end ctor 

   // copy constructor
   AppConfig(const AppConfig &rhs) {
      appName = rhs.appName;
      dbPath = rhs.dbPath;
      dbDoorStateTable = rhs.dbDoorStateTable;
      dbSensorTable = rhs.dbSensorTable;
      dIos = rhs.dIos;
      loopTimeMS = rhs.loopTimeMS;
      fastPwmHz = rhs.fastPwmHz;
      slowPwmHz = rhs.slowPwmHz;
      homingPwmHz = rhs.homingPwmHz;
      sensorReadIntervalSec = rhs.sensorReadIntervalSec;
   } // end ctor

   // assignment operator 
   AppConfig &operator=(const AppConfig &rhs) {
      appName = rhs.appName;
      dbPath = rhs.dbPath;
      dbDoorStateTable = rhs.dbDoorStateTable;
      dbSensorTable = rhs.dbSensorTable;
      dIos = rhs.dIos;
      loopTimeMS = rhs.loopTimeMS;
      fastPwmHz = rhs.fastPwmHz;
      slowPwmHz = rhs.slowPwmHz;
      homingPwmHz = rhs.homingPwmHz;
      sensorReadIntervalSec = rhs.sensorReadIntervalSec;
      return *this;
   } // assignment operator

   /// \brief function to initial struct members 
   void Initialize() {
      appName = "";
      dbPath = "";
      dbDoorStateTable = "";
      dbSensorTable = "";
      dIos.clear();
      loopTimeMS = 0;
      fastPwmHz = 0;
      slowPwmHz = 0;
      homingPwmHz = 0;
      sensorReadIntervalSec = 0;
   } // end Initialize

   string appName;               /// application name 
   string dbPath;                /// the full path to the shared databased 
   string dbDoorStateTable;      /// name of the door state table 
   string dbSensorTable;         /// name of the sensor reading table 
   vector<IoConfig> dIos;        /// a list of the digital io points 
   int loopTimeMS;               /// the program's read input loop time in ms
   int fastPwmHz;                /// fast door pwm hertz, used for opening
   int slowPwmHz;                /// slow door pwm hertz, used for closing 
   int homingPwmHz;              /// very slow door pwm hertz, homing and jogging 
   int sensorReadIntervalSec;    /// the for all sensors read interval in seconds 
}; // end struct 


#endif // end header guard