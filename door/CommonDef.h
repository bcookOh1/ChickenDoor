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
#include <filesystem>


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
const string CONFIG_NIGHT_LIGHT_LEVEL = "ChickenCoop.night_light_level";
const string CONFIG_MORNING_LIGHT_LEVEL = "ChickenCoop.morning_light_level";
const string CONFIG_SENSOR_READ_INTERVAL_SEC = "ChickenCoop.sensor_read_interval_sec";

// string values for digital io type 
const string DIGITAL_INPUT_STR = "input";
const string DIGITAL_OUTPUT_STR = "output";

// string values for digital input resistor mode 
const string INPUT_RESISTOR_NONE_STR = "none";
const string INPUT_RESISTOR_PULLDOWN_STR = "pulldown";
const string INPUT_RESISTOR_PULLUP_STR = "pullup";

// light level constants used in the state machine and manual mode
const float NIGHT_LIGHT_LEVEL_THRESHOLD = 2000.0f; 
const float MORNING_LIGHT_LEVEL_THRESHOLD = 3000.0f; 
const float LIGHT_LEVEL_MANUAL_UP = 10001.1f; 
const float LIGHT_LEVEL_MANUAL_DOWN = -1.0f; 


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
      Initialize();
   } // end ctor 

   // copy constructor
   AppConfig(const AppConfig &rhs) {
      appName = rhs.appName;
      dbPath = rhs.dbPath;
      dbDoorStateTable = rhs.dbDoorStateTable;
      dbSensorTable = rhs.dbSensorTable;
      dIos = rhs.dIos;
      loopTimeMS = rhs.loopTimeMS;
      pwmHzFast = rhs.pwmHzFast;
      pwmHzSlow = rhs.pwmHzSlow;
      pwmHzHoming = rhs.pwmHzHoming;
      sensorReadIntervalSec = rhs.sensorReadIntervalSec;
      morningLight = rhs.morningLight;
      nightLight = rhs.nightLight;
   } // end ctor

   // assignment operator 
   AppConfig &operator=(const AppConfig &rhs) {
      appName = rhs.appName;
      dbPath = rhs.dbPath;
      dbDoorStateTable = rhs.dbDoorStateTable;
      dbSensorTable = rhs.dbSensorTable;
      dIos = rhs.dIos;
      loopTimeMS = rhs.loopTimeMS;
      pwmHzFast = rhs.pwmHzFast;
      pwmHzSlow = rhs.pwmHzSlow;
      pwmHzHoming = rhs.pwmHzHoming;
      sensorReadIntervalSec = rhs.sensorReadIntervalSec;
      morningLight = rhs.morningLight;
      nightLight = rhs.nightLight;
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
      pwmHzFast = 0;
      pwmHzSlow = 0;
      pwmHzHoming = 0;
      sensorReadIntervalSec = 0;
      morningLight = 0.0f;
      nightLight = 0.0f;
   } // end Initialize

   string appName;               /// application name 
   string dbPath;                /// the full path to the shared databased 
   string dbDoorStateTable;      /// name of the door state table 
   string dbSensorTable;         /// name of the sensor reading table 
   vector<IoConfig> dIos;        /// a list of the digital io points 
   int loopTimeMS;               /// the program's read input loop time in ms
   int pwmHzFast;                /// fast door pwm hertz, used for opening
   int pwmHzSlow;                /// slow door pwm hertz, used for closing 
   int pwmHzHoming;              /// very slow door pwm hertz, homing and jogging 
   int sensorReadIntervalSec;    /// for all sensors, the read interval in seconds 
   float morningLight;           /// light threshold to open the door in morning  
   float nightLight;             /// light threshold to close the door at night  
}; // end struct 


// constant, enum, and string/cout utils for user selected mode 

// this is where the php web page write the mode file 
const filesystem::path MODE_FILE{ "/home/bjc/coop/exe/coop_mode.txt" };

// the 3 mode plus an undefined used as a default 
enum class UserSelectedMode : unsigned {
   Undefined,
   Manual_Up,
   Manual_Down,
   Auto_Mode
}; // end enum

// to string utility
string UserSelectedModeToString(UserSelectedMode usm){
   string ret;

   switch (usm) {
   case UserSelectedMode::Undefined:
      ret = "Undefined";
      break;
   case UserSelectedMode::Manual_Up:
      ret = "Manual_Up";
      break;
   case UserSelectedMode::Manual_Down:
      ret = "Manual_Down";
      break;
   case UserSelectedMode::Auto_Mode:
      ret = "Auto_Mode";
      break;
   } // end switch

   return ret;
} // end UserSelectedModeToString

// util for stream/cout 
ostream &operator<<(ostream &out, UserSelectedMode usm) {
   out << UserSelectedModeToString(usm);
   return out;
} // end operator



#endif // end header guard