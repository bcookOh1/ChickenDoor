


#include <filesystem>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>
#include <functional> 
#include <ctime>
#include <boost/coroutine2/all.hpp>

#include "CommonDef.h"
#include "wiringPi.h"
#include "ParseCommandLine.h"
#include "ReadConfigurationFile.h"
#include "DigitalIO.h"
#include "Rp4bPwm.h"
#include "Util.h"
#include "UpdateDatabase.h"
#include "StateMachine.hpp"
#include "Camera.h"
#include "PiTempReader.h"
#include "Tsl2591Reader.h"
#include "Si7021Reader.h"
#include "PrintUtils.h"
#include "UserInputIPC.h"
#include "DateTimeUtils.h"
#include "SunriseSunset.h"

using namespace std;
using Ccsm = sm_chicken_coop;
using namespace boost::coroutines2;
namespace sml = boost::sml;
namespace fs = std::filesystem;

// entry point for the program
// usage: ./coop [-h] -c <config_file> 
// -h, optional, shows this help text, if included other arguments are ignored
// -c <config_file>, a json file with the configuration 
// Note: a space is required between -c and the config file 
// example: sudo ./coop -c config_1.json 
// note: if user types p <enter> enable PrintLn()
// note: if user types s <enter> disable PrintLn()
// note: if user types q <enter> quit program
// note: if user types u <enter> to manually raise the door 
// note: if user types d <enter> to manually lower the door 
// note: if user types a <enter> to resume auto mode 
int main(int argc, char* args[]){

   cout << "coop started with " << argc << " params" <<  endl;

   // get and clean the exe name, used for process count
   string exeName;
   fs::path exePath(args[0]);
   if(exePath.has_stem()) {
     exeName = exePath.stem().string();
   }
   else {
     exeName = exePath.filename().string();
   } // end if 

   // check and convert command line params   
   ParseCommandLine pcl;
   int result = pcl.Parse(argc, args); 
   if(result != 0) {
      cout << "command line error: " << pcl.GetErrorString() <<  endl;
      return 0;
   } // end if 

   if(pcl.GetHelpFlag() == true){
      cout << pcl.GetHelpString() << endl;
      return 0;
   } // end if 

   // class to read the json config file, from command line filename 
   ReadConfigurationFile rcf;
   rcf.SetConfigFilename(pcl.GetConfigFile());

   result = rcf.ReadIn();
   if(result != 0) {
      cout << "configuration file read-in error: " <<  rcf.GetErrorStr() << endl;
      return 0;
   } // end if 

   // an IPC to control printing
   SmallIpc sipc;
   sipc.Writer(0); // start PrintLn() disabled

   // set the config in the AppConfig class, this is a a simple container class  
   // passed to to other classes in the app
   AppConfig ac = rcf.GetConfiguration();

   // set the sqlite3 file path in the database class
   UpdateDatabase udb;
   udb.SetDbFullPath(ac.dbPath);
   udb.SetDoorStateTableName(ac.dbDoorStateTable);
   udb.SetSensorDataTableName(ac.dbSensorTable);

   // make a digial io class and configure digital io points
   DigitalIO digitalIo;
   digitalIo.SetIoPoints(ac.dIos);
   digitalIo.ConfigureHardware();

   // setup empty IoValue map used for algo data 
   IoValues ioValues = MakeIoValuesMap(ac.dIos);

   Rp4bPwm pwm(PwmNumber::Pwm1);
   pwm.SetFrequenceHz(ac.pwmHzHoming); 
   pwm.SetDutyCyclePercent(50);
   pwm.Enable(false);

   ioValues["enable"] = 0u;     // 0 = on at stepper controller
   ioValues["direction"] = 1u;  // 1 = off at stepper controller
   ioValues["r1"] = 0u;
   ioValues["r2"] = 0u;
   ioValues["r3"] = 0u;
   digitalIo.SetOutputs(ioValues);

   // read all now so the eInit{} in state machine has fresh data 
   result = digitalIo.ReadAll(ioValues);
   if(result != 0){
      PrintLn((boost::format{ "read gpio error: %1%" } % digitalIo.GetErrorStr()).str());
   } // end if 

   // reader for pi temp
   PiTempReader pitr;

   // read the temp once at the start so the temperature var is valid
   string temperature;
   result = pitr.RunTask();
   if(result != 0){
      cout << pitr.GetError() << endl;
      return 0;
   }
   else {
      temperature = pitr.GetData();
   } // end if 

   // readers for ambient sensor temp, humidity, and light
   Si7021Reader si7021r;
   Tsl2591Reader tsl2591r;
   float light = 0.0f;
   string lightStr = "0.0";
   SmoothingFilter<float> lightQueue(7); // try the queue size of 7 

   NoBlockTimer nbTimer;
   Ccsm ccsm(ioValues, ac, pwm, nbTimer, light);

   // lambda as callback from the state machine to set a door_state table
   // see int SetStateMachineCB() im StateMachine.hpp
   auto SetDoorStateTableFromSM = [&] (DoorState ds){
       UpdateDoorStateDB(ds, udb, lightStr, temperature);
   }; // end lambda

   // set the callback from main SetOutputFromSM() into the statemachine.hpp SetStateMachineCB()
   ccsm.SetStateMachineCB(std::bind(SetDoorStateTableFromSM, std::placeholders::_1));

   sml::sm<Ccsm> sm(ccsm);
   bool ready = false;

   // move off Idle1 state
   sm.process_event(eInit{});
   digitalIo.SetOutputs(ioValues); // must follow since eInit sets the direction and enable outputs

   Camera cam;
   bool cameraInuse = false;
   
   // to allow user to enable/disable printing 
   WatchConsole wc;
   wc.Setup();

   // class to read the webpage user selected mode through ipc 
   UserInputIPC usmIpc;

   // mode is user selected mode from either cin or the web page through IPC 
   UserInput mode = UserInput::Auto_Mode;
   bool lightDataAvaliable = false;

   // setr address struct from the configuration
   Address address{ac.houseNumber, ac.street, ac.city, ac.state, ac.zipCode};

   SunriseSunset srss{address, Time2Check4NewSunriseSunset};
   std::function<void(coroutine<SunriseSunsetStatus>::push_type &)> fn = 
      std::bind(&SunriseSunset::FetchTimes, &srss, std::placeholders::_1);

   // declare the coroutine GetSunriseSunsetTimes
   coroutine<SunriseSunsetStatus>::pull_type GetSunriseSunsetTimes{ fn };


   while(true) {

      // if user types p <enter> enable PrintLn()
      // if user types s <enter> disable PrintLn()
      // if user types q enter quit program
      // if user types u enter manual mode, door up
      // if user types d  enter manual mode, door down
      // if user types a auto mode
      if(wc.CheckForInput() == true) {
         string in = wc.GetInput();
         if(in[0] == 'q') break;

         if(in[0] == 'p')  sipc.Writer(1);
         if(in[0] == 's')  sipc.Writer(0);
         
         if(in[0] == 'a')  mode = UserInput::Auto_Mode; 
         if(in[0] == 'u')  mode = UserInput::Manual_Up;
         if(in[0] == 'd')  mode = UserInput::Manual_Down;
         if(in[0] == 'c')  mode = UserInput::Take_Picture;

      } // end if 

      // look for a new mode selection from the webpage 
      if(usmIpc.NewModeFile() == true) {
         if(usmIpc.ReadMode() == 0) {
            mode = usmIpc.GetMode();
            usmIpc.DeleteModeFile();
            PrintLn((boost::format{ "new user mode: %1%" } % UserInputToString(mode)).str());
         }
         else {
            PrintLn("read mode file error");
         } // end if 
      } // end if 

      auto status = GetSunriseSunsetTimes().get();
      if (status == SunriseSunsetStatus::SunRiseSetComplete) {
         auto times = srss.GetTimes();
         times.Print();
      }
      else if (status == SunriseSunsetStatus::Error) {
         cout << "error" << srss.GetError() << endl << endl;
      }// end if 
      
      result = digitalIo.ReadAll(ioValues);
      if(result != 0){
         PrintLn((boost::format{ "read gpio error: %1%" } % digitalIo.GetErrorStr()).str());
         break;
      } // end if 

      // set the events to the state machine
      if(ready == false){
         sm.process_event(eStartUp{});
      } 
      else if(lightDataAvaliable == true){
         sm.process_event(eOnTime{});
      } // end if 

      digitalIo.SetOutputs(ioValues);

      // bcook 6/14/2020 ignore this error for now
      ////////////////////////////////////////////////////////////////
      // if(sm.is(sml::state<Failed>) == true) {cout << "failed state" << endl; break;}

      // 
      if(sm.is(sml::state<HomingComplete>) == true) 
         ready = true;

      if(sm.is(sml::state<ObstructionDetected>) == true) {
         PrintLn("main: ObstructionDetected");
      } // end if 

      if(mode == UserInput::Take_Picture) {
         PrintLn("main: Take Picture");
         
         if(cameraInuse == false) {
            cameraInuse = true;
            cam.StillAsync();
         } // end if

         // do only once if user selects take picture
         // so set to UserInput::Undefined 
         mode = UserInput::Undefined;
      } // end if 

      if(cameraInuse == true) {
         if(cam.IsDone()){
            cameraInuse = false;
         } // end if 
      } // end if 

      // read PI temp every n seconds
      if(pitr.GetStatus() == ReaderStatus::NotStarted){
         pitr.ReadAfterSec(ac.sensorReadIntervalSec);
      }
      else if(pitr.GetStatus() == ReaderStatus::Complete){
         temperature = pitr.GetData();
         pitr.ResetStatus();
      }
      else if(pitr.GetStatus() == ReaderStatus::Error) {
         cout << pitr.GetError() << endl;
         pitr.ResetStatus();
      } // end if 
 
      // read Si7021 temp and humidity every n seconds
      if(si7021r.GetStatus() == ReaderStatus::NotStarted){
         si7021r.ReadAfterSec(ac.sensorReadIntervalSec);
      }
      else if(si7021r.GetStatus() == ReaderStatus::Complete){
         Si7021Data data = si7021r.GetData();
         si7021r.ResetStatus();

         string lightUnits = "lx";

         // write sensor data to db 
         int sensorReadResult = udb.AddOneSensorDataRow(GetSqlite3DateTime(),  
                                                        data.temperature,
                                                        data.TemperatureUnits,
                                                        data.humidity,
                                                        data.humidityUnits,
                                                        lightStr, lightUnits); 
         if(sensorReadResult == -1) {
            cout << udb.GetErrorStr() << endl;
         } // end if 

      }
      else if(si7021r.GetStatus() == ReaderStatus::Error) {
         cout << si7021r.GetError() << endl;
         si7021r.ResetStatus();
      } // end if 

      // read Tsl2591 light level every n seconds
      if(tsl2591r.GetStatus() == ReaderStatus::NotStarted){
         tsl2591r.ReadAfterSec(ac.sensorReadIntervalSec);
      }
      else if(tsl2591r.GetStatus() == ReaderStatus::Complete){
         Tsl2591Data data = tsl2591r.GetData();
         tsl2591r.ResetStatus();

         ////////////////////////////////////////////////
         // set the average light level in manual mode,
         // this bypasses the average but still maintains the 
         // queue so a return to auto mode moves the door
         // baased on the current light levels 
         lightQueue.Add(data.lightLevel);

         if(mode == UserInput::Manual_Up){ // raise
             light =  LIGHT_LEVEL_MANUAL_UP;
         }
         else if(mode == UserInput::Manual_Down){ // lower
             light = LIGHT_LEVEL_MANUAL_DOWN;
         }
         else { // resume auto or undefined so also resume auto
             light = lightQueue.GetFilteredValue();
         } // end if 

         lightStr = str(format("%.1f") %  light);
         lightDataAvaliable = true;

      }
      else if(tsl2591r.GetStatus() == ReaderStatus::Error) {
         cout << tsl2591r.GetError() << endl;
         tsl2591r.ResetStatus();
      } // end if 
            
      this_thread::sleep_for(chrono::milliseconds(ac.loopTimeMS));
   } // end while 

   // all off  
   pwm.Enable(false);
   ioValues["direction"] = 1u;
   digitalIo.SetOutputs(ioValues);

   // restore cin to blocking mode 
   wc.Close();

   return 0;
} // end main

