// file: ParseCommandLine.h header for ParseCommandLine
// author: Bennett Cook
// date: 07-20-2029
// 

// header guards 
#ifndef PARSECOMMANDLINE_H
#define PARSECOMMANDLINE_H

#include <string>
#include <vector>

#include "CommonDef.h"
#include "Util.h"

using namespace std; 

/// \const command line flag constants
const string COMMAND_LINE_HELP_FLAG = "-h";
const string COMMAND_LINE_CONFIG_FILE_FLAG = "-c";
const string COMMAND_LINE_SILENT_FLAG = "-s";
const int MINIMUM_ARGUMENTS = 2;

/// \const constants for defaults 
const bool HELP_FLAG_DEFAULT = false;
const bool SILENT_FLAG_DEFAULT = false;

/// \const the command line help string 
const string COMMAND_LINE_HELP_STRING =
"Usage: ./coop [-h] -c <config_file> [-s] \n"
"-h, optional, shows this help text, if included other arguments are ignored\n"
"-c <config_file>, a json file with the configuration \n"
"-s, optional, the io and state information is not printed but error messages are \n"
"Note: a space is required between -c and the config file \n"
"example:\n./coop -c config_1.json -s ";

/// \class ParseCommandLine files ParseCommandLine.h/cpp
/// \brief class to parse command line with get.. functions 
class ParseCommandLine {

public:

  ParseCommandLine();
  ~ParseCommandLine();

  /// parse the command line params passed in from the main.cpp 
  /// \param argc number of command line params
  /// \param argv an array of chars 
  int Parse(int argc, char** argv);

  bool GetHelpFlag() { return _help; }
  string GetConfigFile() { return _configFile; }
  bool GetSilentFlag() { return _silent; }

  string GetErrorString() { return _errorStr; }
  string GetHelpString() { return COMMAND_LINE_HELP_STRING; }

private: 

  // member vars, set from the command line  
  bool _help;          //!< \var help flag
  bool _silent;        //!< \var silent flag
  string _configFile;  //!< \var the configuration file name
  string _errorStr;    //!< \var the parsing error string 

};// end class

#endif // end header guard

