



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
#include "Camera.h"
#include "PCF8591Reader.h"
#include "PrintUtils.h"


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
   ioValues["sun"] = 0;
   digitalIo.SetOutputs(ioValues);

   NoBlockTimer nbTimer;

   // initialize with night level 
   float light = 1024.0f; // sensor is high at night low in day
   PCF8591Reader pcf8591r;
   pcf8591r.SetChannelAndConversion(PCF8591_AI_CHANNEL::Ai0, [=](float in){return (in * 4.0f) + 0.0f;});

   Ccsm ccsm(ioValues, ac, pwm, nbTimer, light);
   sml::sm<Ccsm> sm(ccsm);

   // move off Idle1 states
   sm.process_event(eInit{});
   bool ready = false;

   Camera cam;
   bool cameraInuse = false;

   // used as edge detect on ioValues["toggle"]
   // so no need to hold the button down 
   unsigned toggleLast = 0;
   
   // allow user to enable/disable printing 
   SmallIpc sipc;
   ConsoleWatcher wc;
   wc.Setup();

   while(true) {

      // if user types 'c' enter enable PrintLn()
      if(wc.CheckForInput() == 1) {
         string in = wc.GetInput();
         sipc.Writer(in[0] == 'c' ? 1 : 0);
      } // end if 
      
      int result = digitalIo.ReadAll(ioValues);
      if(result != 0){
        cout << "read gpio error: " << digitalIo.GetErrorStr() << endl; 
        break;
      } // end if 

      sm.process_event(eOnTime{});

      // rising edge detect switches the 'sun' output
      if(ioValues["toggle"] == 1) {
         if(toggleLast == 0){ 
            ioValues["sun"] = (ioValues["sun"] == 1 ? 0 : 1);
         } // end if 
      } // end if 
      toggleLast = ioValues["toggle"];

      digitalIo.SetOutputs(ioValues);
      
      // printInputs(digitalIo, ioValues);

      if(sm.is(sml::state<Failed>) == true) break;
      if(sm.is(sml::state<Closed>) == true) ready = true;
      if(sm.is(sml::state<ObstructionDetected>) == true) {
         cout << "main: ObstructionDetected" << endl;
         
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
      
      // use features when the door is ready 
      if(ready) {
 
         // read PCF8591 analog 
         if(pcf8591r.GetStatus() == ReaderStatus::NotStarted){
            pcf8591r.ReadAfterSec(10);
         }
         else if(pcf8591r.GetStatus() == ReaderStatus::Complete){
            light = pcf8591r.GetData();
            cout << "light: " << light << endl;
            pcf8591r.ResetStatus();
         }
         else if(pcf8591r.GetStatus() == ReaderStatus::Error) {
            cout << pcf8591r.GetError() << endl;
            pcf8591r.ResetStatus();
            break;
         } // end if 

     } // end if 

      this_thread::sleep_for(chrono::milliseconds(ac.loopTimeMS));
   } // end while 

   // restore cin to blocking mode 
   wc.Quit();

   pwm.Enable(false);

   ioValues["enable"] = 0;
   ioValues["direction"] = 0;
   ioValues["sun"] = 0;
   digitalIo.SetOutputs(ioValues);

   return 0;
} // end main