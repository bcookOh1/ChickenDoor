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

   bool NewModeFile();
   int ReadMode(); 
   bool DeleteModeFile();
   UserInput GetMode();

private: 
   char _mode;
}; // end 

