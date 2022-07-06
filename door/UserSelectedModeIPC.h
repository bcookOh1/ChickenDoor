#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include "CommonDef.h"

using namespace std;
namespace fs = std::filesystem;


class UserSelectedModeIPC {
public:
   UserSelectedModeIPC(); 
   ~UserSelectedModeIPC();

   bool NewModeFile();
   int ReadMode(); 
   bool DeleteModeFile();
   UserSelectedMode GetMode();

private: 
   char _mode;
}; // end 

