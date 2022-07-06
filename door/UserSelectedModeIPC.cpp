#include "UserSelectedModeIPC.h"


UserSelectedModeIPC::UserSelectedModeIPC() : 
   _mode{ ' ' } {
} // end ctor 


UserSelectedModeIPC::~UserSelectedModeIPC() {
} // end dtor 


bool UserSelectedModeIPC::NewModeFile() {
   return filesystem::exists(MODE_FILE);
} // end NewModeFile


int UserSelectedModeIPC::ReadMode() {
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


bool UserSelectedModeIPC::DeleteModeFile() {
   return fs::remove(MODE_FILE);
} // DeleteModeFile


UserSelectedMode UserSelectedModeIPC::GetMode() {
   UserSelectedMode ret{ UserSelectedMode::Undefined };

   switch (_mode) {
   case 'u':
      ret = UserSelectedMode::Manual_Up;
      break;
   case 'd':
      ret = UserSelectedMode::Manual_Down;
      break;
   case 'a':
      ret = UserSelectedMode::Auto_Mode;
      break;
   } // end switch

   return ret;
} // end GetMode
