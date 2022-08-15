#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include "CommonDef.h"

using namespace std;
namespace fs = std::filesystem;


class UserInputIPC {
public:
   UserInputIPC(); 
   ~UserInputIPC();

   bool NewFile();
   int ReadUserInput(); 
   bool DeleteFile();
   UserInput GetUserInput();

private: 
   char _userInput;
}; // end 

