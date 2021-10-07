// file: ParseCommandLine.cc implementation for ParseCommandLine
// author: Bennett Cook
// date: 07-20-2020
// 

#include "ParseCommandLine.h"


/// \brief default constructor  
ParseCommandLine::ParseCommandLine() {
  _help = HELP_FLAG_DEFAULT;
  _silent = SILENT_FLAG_DEFAULT;
} // end ctor 


/// \brief do nothing destructor 
ParseCommandLine::~ParseCommandLine() {
} // dtor 


/// \brief parse the command line params passed in from the main.cc 
/// \param argc number of command line params
/// \param argv an array of chars
/// \return 0 success
/// \return -1 too few command line params
/// \remark example: ./gdoor -c config_1.json -s
int ParseCommandLine::Parse(int argc, char **argv) {

  int ret = 0;

  // if no params set then show help 
  if (1 == argc) {
    _help = true;
    return 0;
  } // end if 

  // assume 2 minimum args 
  if (argc < MINIMUM_ARGUMENTS)
    return -1;

  // parse each command line arg, skip argv[0] which is the exec file name 
  for (auto i = 1; i < argc; i++) {
    string arg(argv[i]);

    if (arg == COMMAND_LINE_HELP_FLAG) {
      _help = true;
    }
    else if (arg == COMMAND_LINE_SILENT_FLAG) {
      _silent = true;
    }
    else if (arg == COMMAND_LINE_CONFIG_FILE_FLAG) {

      // must have arg after flag 
      if (i + 1 < argc) {

        _configFile = argv[i + 1];
        i++; // increment since paired arg  
      }
      else {
        _errorStr += "\nconfig file missing";
        ret = -1;
      } // end if 

    } // end if 

  } // end for

  return ret;
} // end SetCommandLine

