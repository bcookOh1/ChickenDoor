
#include "UserInputIPC.h"


UserInputIPC::UserInputIPC() : 
   _mode{' '} {
} // end ctor 


UserInputIPC::~UserInputIPC() {
} // end dtor 


bool UserInputIPC::NewModeFile() {
   return fs::exists(MODE_FILE);
} // end NewModeFile


int UserInputIPC::ReadMode() {
   int ret = -1;
   string line;

   // open a file in read mode.
   ifstream infile(MODE_FILE.c_str());
   if (infile.fail()) {
      return -1;
   } // end if

   getline(infile, line, '\r');
   infile.close();

   if (line.size() >= 1) {
      _mode = line[0];
      ret = 0;
   }
   else {
      ret = -1;
   } // end if 

   return ret;
} // end GetMode 


bool UserInputIPC::DeleteModeFile() {
   return fs::remove(MODE_FILE);
} // DeleteModeFile


UserInput UserInputIPC::GetMode() {
   UserInput ret{ UserInput::Undefined };

   switch (_mode) {
   case 'u':
      ret = UserInput::Manual_Up;
      break;
   case 'd':
      ret = UserInput::Manual_Down;
      break;
   case 'a':
      ret = UserInput::Auto_Mode;
      break;
   } // end switch

   return ret;
} // end GetMode
