



// #include <boost/asio.hpp>
// #include <boost/bind/bind.hpp>
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

// for direct control w/o WiringPI see sysfs section: https://elinux.org/RPi_GPIO_Code_Samples#bcm2835_library

using namespace std;
namespace filesys = boost::filesystem;
namespace sml = boost::sml;
using Ccsm = sm_chicken_coop;

//g++ -g -opwm main.cpp Rp4bPwm.cpp -lwiringPi -lpthread
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

   // set the config in the AppConfig class, this is a a simple container class  
   // passed to to other classes in the app
   AppConfig ac = rcf.GetConfiguration();

   // make a digial io class and configure digital io points
   DigitalIO digitalIo;
   digitalIo.SetIoPoints(ac.dIos);
   digitalIo.ConfigureHardware();

   // setup empty IoValue map used for algo data 
   IoValues ioValues = MakeIoValuesMap(ac.dIos);

   // lambda to print io in single line 
   auto printInputs = [](DigitalIO &digitalIo, IoValues &ioValues){
      digitalIo.ReadAll(ioValues);
      cout << IoToLine(ioValues);
   }; // end lambda

   Rp4bPwm pwm(PwmNumber::Pwm1);
   pwm.SetFrequenceHz(ac.fastPwmHz); 
   pwm.SetDutyCyclePercent(50);
   pwm.Enable(false);

   ioValues["enable"] = 1;
   ioValues["direction"] = 0;
   digitalIo.SetOutputs(ioValues);

   NoBlockTimer nbTimer;
   float light = 105.0f;

   Ccsm ccsm(ioValues, ac, pwm, nbTimer, light);
   sml::sm<Ccsm> sm(ccsm);

   // move off Idle1 states
   sm.process_event(eInit{});

   while(true){
      
      digitalIo.ReadAll(ioValues);
      sm.process_event(eOnTime{});
      digitalIo.SetOutputs(ioValues);
      
      //printInputs(digitalIo, ioValues);

      if(sm.is(sml::state<Failed>) == true) break;
      if(sm.is(sml::state<Closed>) == true) break;

      this_thread::sleep_for(chrono::milliseconds(ac.loopTimeMS));
   } // end while 

   pwm.Enable(false);

   ioValues["enable"] = 0;
   ioValues["direction"] = 0;
   digitalIo.SetOutputs(ioValues);

   return 0;
} // end main