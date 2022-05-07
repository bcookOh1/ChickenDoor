


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
#include "StateMachine.hpp"
#include "Camera.h"
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
// note: if user types q enter quit program
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

   // temp/humidity and light sensor readers
   Tsl2591Reader tsl2591r;
   Si7021Reader si7021r;
   float light = 0.0f;

   NoBlockTimer nbTimer;
   Ccsm ccsm(ioValues, ac, pwm, nbTimer, light);

   // lambda as callback from the state machine to set a gpio output
   // see int SetStateMachineCB() im StateMachine.hpp
   auto SetOutputFromSM = [&] (string name, unsigned value){
      ioValues[name] = value;
      cout << "cb: " << name << "=" << value << endl;  
   }; // end lambda

   // set the the callback from main SetOutputFromSM() into the statemachine.hpp SetStateMachineCB()
   ccsm.SetStateMachineCB(std::bind(SetOutputFromSM, std::placeholders::_1, std::placeholders::_2 ));

   sml::sm<Ccsm> sm(ccsm);
   bool ready = false;

   // move off Idle1 states
   sm.process_event(eInit{});
   digitalIo.SetOutputs(ioValues); // must follow since eInit sets the direction and enable

   Camera cam;
   bool cameraInuse = false;
   
   // to allow user to enable/disable printing 
   WatchConsole wc;
   wc.Setup();

   while(true) {

      // if user types 'c' <enter> enable PrintLn()
      // if user types q enter quit program
      if(wc.CheckForInput() == true) {
         string in = wc.GetInput();
         if(in[0] == 'q') break;
         sipc.Writer(in[0] == 'c' ? 1 : 0);
      } // end if 
      
      result = digitalIo.ReadAll(ioValues);
      if(result != 0){
         PrintLn((boost::format{ "read gpio error: %1%" } % digitalIo.GetErrorStr()).str());
         break;
      } // end if 

      sm.process_event(eOnTime{});

      digitalIo.SetOutputs(ioValues);

      if(sm.is(sml::state<Failed>) == true) {cout << "failed state" << endl; break;}
      if(sm.is(sml::state<Open>) == true) ready = true;

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
      

      // read Si7021 temp and humidity every n seconds
      if(si7021r.GetStatus() == ReaderStatus::NotStarted){
         si7021r.ReadAfterSec(2);
      }
      else if(si7021r.GetStatus() == ReaderStatus::Complete){
         Si7021Data data = si7021r.GetData();
         si7021r.ResetStatus();

         cout << "si7021: " << data.temperature << data.TemperatureUnits
                            << ", " << data.humidity << data.humidityUnits << endl; 

      }
      else if(si7021r.GetStatus() == ReaderStatus::Error) {
         cout << si7021r.GetError() << endl;
         si7021r.ResetStatus();
      } // end if 

      // use features when the door is ready (homed) 
      if(ready) {

         // read Tsl2591 light level every n seconds
         if(tsl2591r.GetStatus() == ReaderStatus::NotStarted){
            tsl2591r.ReadAfterSec(2);
         }
         else if(tsl2591r.GetStatus() == ReaderStatus::Complete){
            Tsl2591Data data = tsl2591r.GetData();
            tsl2591r.ResetStatus();

            light = data.lightLevel;
            cout << "tsl2591: " << data.lightLevel << ", " << data.rawlightLevel << endl; 
         }
         else if(tsl2591r.GetStatus() == ReaderStatus::Error) {
            cout << tsl2591r.GetError() << endl;
            tsl2591r.ResetStatus();
         } // end if 
            
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

