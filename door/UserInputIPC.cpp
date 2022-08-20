
#include "UserInputIPC.h"


UserInputIPC::UserInputIPC() : 
   _userInput{' '} {
} // end ctor 


UserInputIPC::~UserInputIPC() {
} // end dtor 


bool UserInputIPC::NewFile() {
   return fs::exists(MODE_FILE);
} // end NewFile


int UserInputIPC::ReadUserInput() {
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
      _userInput = line[0];
      ret = 0;
   }
   else {
      ret = -1;
   } // end if 

   return ret;
} // end ReadUserInput 


bool UserInputIPC::DeleteFile() {
   return fs::remove(MODE_FILE);
} // DeleteFile


UserInput UserInputIPC::GetUserInput() {
   UserInput ret{ UserInput::Undefined };

   switch (_userInput) {
   case 'u':
      ret = UserInput::Manual_Up;
      break;
   case 'd':
      ret = UserInput::Manual_Down;
      break;
   case 'a':
      ret = UserInput::Auto_Mode;
      break;
   case 'c':
      ret = UserInput::Take_Picture;
      break;
   } // end switch

   return ret;
} // end GetUserInput
