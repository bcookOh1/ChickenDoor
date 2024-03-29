


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
   udb.SetSunDataTableName(ac.dbSunDataTable);

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
   Ccsm ccsm(ioValues, ac, pwm, nbTimer);

   // used in the decision section in while() to document 
   // what decision was taken, dec is added to the door state table
   Decision dec = Decision::Undefined;

   // lambda as callback from the state machine to set a door_state table
   // see int SetStateMachineCB() im StateMachine.hpp
   auto SetDoorStateTableFromSM = [&] (DoorState ds){
      string decStr = DecisionToString(dec); 
      UpdateDoorStateDB(ds, udb, lightStr, temperature, decStr);
   }; // end lambda

   // set the callback from main SetOutputFromSM() into the statemachine.hpp SetStateMachineCB()
   ccsm.SetStateMachineCB(std::bind(SetDoorStateTableFromSM, std::placeholders::_1));

   sml::sm<Ccsm> sm(ccsm);
   bool doorHomed = false;

   // move off Idle1 state
   sm.process_event(eInit{});
   digitalIo.SetOutputs(ioValues); // must follow since eInit sets the direction and enable outputs

   Camera cam;
   bool cameraInuse = false;
   
   // to allow user to enable/disable printing 
   WatchConsole wc;
   wc.Setup();

   // class to read the webpage user selected mode through ipc 
   UserInputIPC uiIpc;

   // mode and takePicture are user input from either cin or the web page through IPC 
   UserInput mode = UserInput::Auto_Mode;
   UserInput takePicture = UserInput::Undefined;

   // class to test if day or night
   Daytime daytime{ac.sunriseOffsetMin, ac.sunsetOffsetMin};
   bool daytimeDataAvailable = false;

   // set true when the light averaging is saturated
   bool lightDataAvaliable = false;

   // setr address struct from the configuration
   Address address{ac.houseNumber, ac.street, ac.city, ac.state, ac.zipCode};

   SunriseSunset srss{address, Time2Check4NewSunriseSunset};
   std::function<void(coroutine<SunriseSunsetStatus>::push_type &)> fn = 
      std::bind(&SunriseSunset::FetchTimes, &srss, std::placeholders::_1);

   // declare the coroutine GetSunriseSunsetTimes
   coroutine<SunriseSunsetStatus>::pull_type GetSunriseSunsetTimes{ fn };

   while(true) {

      //////////////////////////////////////////////////////
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
         if(in[0] == 'c')  takePicture = UserInput::Take_Picture;

      } // end if 

      // end command line inputs 
      //////////////////////////////////////////////////////

      //////////////////////////////////////////////////////
      // look for a new mode selection from the webpage 
      if(uiIpc.NewFile() == true) {
         if(uiIpc.ReadUserInput() == 0) {
            auto iuCmd = uiIpc.GetUserInput();
            uiIpc.DeleteFile();
            PrintLn((boost::format{ "new user input: %1%" } % UserInputToString(iuCmd)).str());

            // separate out manual door commands from the take picture command  
            if(iuCmd == UserInput::Take_Picture)
               takePicture = UserInput::Take_Picture;
            else 
               mode = iuCmd;

         }
         else {
            PrintLn("read mode file error");
         } // end if 
      } // end if 

      // end look for a new mode selection from the webpage 
      //////////////////////////////////////////////////////

      //////////////////////////////////////////////////////
      // get sunrise sunset times   
      auto status = GetSunriseSunsetTimes().get();
      if (status == SunriseSunsetStatus::SunRiseSetComplete) {
         auto times = srss.GetTimes();
         
         // save new sun data time to the database
         int sunDataWriteResult = udb.AddOneSunDataRow(GetSqlite3DateTime(),
                                                       Ptime2TmeString(times.rise), 
                                                       Ptime2TmeString(times.set));
         if(sunDataWriteResult == -1) {
            cout << udb.GetErrorStr() << endl;
         } // end if 

         daytime.SetSunriseSunsetTimes(times.rise, times.set);
         daytimeDataAvailable = true;
      }
      else if (status == SunriseSunsetStatus::Error) {
         daytimeDataAvailable = false;
         cout << "error" << srss.GetError() << endl << endl;
      } // end if 

      // end get sunrise sunset times   
      //////////////////////////////////////////////////////


      //////////////////////////////////////////////////////
      /// day night decision
      // priority: user input then sunrise sunset then light sensor,
      // The user commands are from the webpage.
      //  

      DoorCommand dc{DoorCommand::NoChange};
      dec = Decision::Undefined;
      
      if(mode == UserInput::Manual_Up){ // raise
         dc = DoorCommand::Open;
         dec = Decision::Manual_Up;
      }
      else if(mode == UserInput::Manual_Down){ // lower
         dc = DoorCommand::Close;
         dec = Decision::Manual_Down;
      }
      else if(daytimeDataAvailable == true){

         if(daytime.IsDaytime()){
            dc = DoorCommand::Open;
            dec = Decision::Sunrise_W_Offset;
         }
         else {
            dc = DoorCommand::Close;
            dec = Decision::Sunset_W_Offset;
         } // end if

      } 
      else if(lightDataAvaliable == true){

         if(light > ac.morningLight && IsAM()) {
            dc = DoorCommand::Open;
            dec = Decision::AM_Light;
         } // end if 
         
         if(light < ac.nightLight && !IsAM()) { 
            dc = DoorCommand::Close;
            dec = Decision::PM_Light;
         } // end if 

      } // end if 

      /// end day night decision
      ////////////////////////////////////////////////////////////////
      

      ////////////////////////////////////////////////////////////////
      // main control loop, read input solve logic, set outputs
      // there are other input/outputs but the IO is the main things
      // for the state machine 
      result = digitalIo.ReadAll(ioValues);
      if(result != 0){
         PrintLn((boost::format{ "read gpio error: %1%" } % digitalIo.GetErrorStr()).str());
         break;
      } // end if 

      // set the events to the state machine
      if(doorHomed == false){
         sm.process_event(eStartUp{});
      } 
      else if(lightDataAvaliable == true || daytimeDataAvailable == true){
         sm.process_event(eOnTime{dc});
      } // end if 
      // note: do nothing on else 

      digitalIo.SetOutputs(ioValues);

      // end main control loop
      ////////////////////////////////////////////////////////////////


      // bcook 6/14/2020 ignore this error for now
      ////////////////////////////////////////////////////////////////
      // if(sm.is(sml::state<Failed>) == true) {cout << "failed state" << endl; break;}

      // 
      if(sm.is(sml::state<HomingComplete>) == true) 
         doorHomed = true;

      if(sm.is(sml::state<ObstructionDetected>) == true) {
         PrintLn("main: ObstructionDetected");
      } // end if 

      ////////////////////////////////////////////////////////////////
      /// camera
      if(takePicture == UserInput::Take_Picture) {
         PrintLn("main: Take Picture");
         
         if(cameraInuse == false) {
            cameraInuse = true;
            cam.StillAsync();
         } // end if

         // do only once if user selects take picture
         // so set to UserInput::Undefined 
         takePicture = UserInput::Undefined;
      } // end if 

      if(cameraInuse == true) {
         if(cam.IsDone()){
            cameraInuse = false;
         } // end if 
      } // end if 

      /// end camera
      ////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////
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

      // end read PI temp every n seconds
      ////////////////////////////////////////////////////////////////
 
      ////////////////////////////////////////////////////////////////
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

      // end read Si7021 temp and humidity every n seconds
      ////////////////////////////////////////////////////////////////

      ////////////////////////////////////////////////////////////////
      // read Tsl2591 light level every n seconds
      if(tsl2591r.GetStatus() == ReaderStatus::NotStarted){
         tsl2591r.ReadAfterSec(ac.sensorReadIntervalSec);
      }
      else if(tsl2591r.GetStatus() == ReaderStatus::Complete){
         Tsl2591Data data = tsl2591r.GetData();
         tsl2591r.ResetStatus();

         lightQueue.Add(data.lightLevel);
         light = lightQueue.GetFilteredValue();

         lightStr = str(format("%.1f") %  light);
         lightDataAvaliable = true;
      }
      else if(tsl2591r.GetStatus() == ReaderStatus::Error) {
         cout << tsl2591r.GetError() << endl;
         tsl2591r.ResetStatus();
      } // end if 

      // end read Tsl2591 light level every n seconds
      ////////////////////////////////////////////////////////////////
            
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

