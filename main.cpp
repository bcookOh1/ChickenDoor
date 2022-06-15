


#include <boost/filesystem.hpp>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

#include "wiringPi.h"
#include "ParseCommandLine.h"
#include "ReadConfigurationFile.h"
#include "DigitalIO.h"
#include "Rp4bPwm.h"
#include "CommonDef.h"
#include "Util.h"
#include "UpdateDatabase.h"
#include "StateMachine.hpp"
#include "Camera.h"
#include "PiTempReader.h"
#include "Tsl2591Reader.h"
#include "Si7021Reader.h"
#include "PrintUtils.h"


using namespace std;
namespace filesys = boost::filesystem;
namespace sml = boost::sml;
using Ccsm = sm_chicken_coop;

// entry point for the program
// usage: ./coop [-h] -c <config_file> 
// -h, optional, shows this help text, if included other arguments are ignored
// -c <config_file>, a json file with the configuration 
// Note: a space is required between -c and the config file 
// example: sudo ./coop -c config_1.json 
// note: if user types 'c' <enter> enable PrintLn()
// note: if user types 'q' <enter> quit program
// note: if user types 'u' <enter> to manually raise the door 
// note: if user types 'd' <enter> to manually lower the door 
// note: if user types 'a' <enter> to resume auto mode 
int main(int argc, char* args[]){

   cout << "coop started with " << argc << " params" <<  endl;

   // get and clean the exe name, used for process count
   string exeName;
   filesys::path exePath(args[0]);
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
   pwm.SetFrequenceHz(ac.homingPwmHz); 
   pwm.SetDutyCyclePercent(50);
   pwm.Enable(false);

   ioValues["enable"] = 1u;     // 1 = off at stepper controller
   ioValues["direction"] = 1u;  // 1 = off at stepper controller
   ioValues["r1"] = 1u;
   ioValues["r2"] = 1u;
   ioValues["r3"] = 1u;
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
   float lightAvg = 0.0f;
   string lightStr = "0.00";
   MovingAverage<float> lightQueue(7);

   NoBlockTimer nbTimer;
   Ccsm ccsm(ioValues, ac, pwm, nbTimer, lightAvg);

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

   // manual mode: 
   // 0 = resume auto mode 
   // 1 = raise door ('u')
   // 2 = lower door ('d')
   int manualMode = 0;

   while(true) {

      // if user types 'c' <enter> enable PrintLn()
      // if user types q enter quit program
      if(wc.CheckForInput() == true) {
         string in = wc.GetInput();
         if(in[0] == 'q') break;
         sipc.Writer(in[0] == 'c' ? 1 : 0);
         
         if(in[0] == 'u')  manualMode = 1;
         if(in[0] == 'd')  manualMode = 2;
         if(in[0] == 'a')  manualMode = 0;

      } // end if 
      
      result = digitalIo.ReadAll(ioValues);
      if(result != 0){
         PrintLn((boost::format{ "read gpio error: %1%" } % digitalIo.GetErrorStr()).str());
         break;
      } // end if 

      sm.process_event(eOnTime{});

      digitalIo.SetOutputs(ioValues);

      // bcook 6/14/2020 ignore this error for tonight
      ////////////////////////////////////////////////////////////////
      // if(sm.is(sml::state<Failed>) == true) {cout << "failed state" << endl; break;}
      // if(sm.is(sml::state<Open>) == true) ready = true;
       ready = true;

      if(sm.is(sml::state<ObstructionDetected>) == true) {
         PrintLn("main: ObstructionDetected");
         
         if(cameraInuse == false) {
            cameraInuse = true;
            cam.StillAsync();
         } // end if

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
         // set the light level in manual mode 
         if(manualMode == 1){ // raise
            light = 2000.0f;
         }
         else if(manualMode == 2){ // lower
            light = 0.0f;
         }
         else { // resume auto
            light = data.lightLevel;
         } // end if 

         lightQueue.Add(light);
         lightAvg = lightQueue.GetAverage();

         lightStr = str(format("%f.1") % lightAvg);

      }
      else if(tsl2591r.GetStatus() == ReaderStatus::Error) {
         cout << tsl2591r.GetError() << endl;
         tsl2591r.ResetStatus();
      } // end if 
            
      this_thread::sleep_for(chrono::milliseconds(ac.loopTimeMS));
   } // end while 

   // all off  
   pwm.Enable(false);
   ioValues["enable"] = 1u;
   ioValues["direction"] = 1u;
   digitalIo.SetOutputs(ioValues);

   // restore cin to blocking mode 
   wc.Close();

   return 0;
} // end main

