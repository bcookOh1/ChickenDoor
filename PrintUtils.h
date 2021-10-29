
#ifndef PRINTUTILS_H
#define PRINTUTILS_H

#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <string>
#include <array>
#include <thread>
#include <future>
#include <chrono>
#include <atomic>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
using namespace boost::interprocess;


const string SharedMemoryName = "CPP";
const string SharedMemoryItem = "PrintEnable";


//*******************************************************************
// variadaic template print functions, two functions are needed, 
// the first vtprint() is the final one the compiler uses, it just
// prints a newline, The second uses the variadatic template. 
// note: these function DO NOT use the shared memory item to 
// enable/disable printing. These always print. 
// usage: vtprint("one", 2, "three", 4);
void vtprint();

template <typename T, typename... Types>
void vtprint(T V1, Types... V2);
//*******************************************


//*******************************************************************
// Note this class sets stdin (and cin) to non-blocking. It restores 
// stdin to blocking mode when Close() is called.
// WatchConsole is intended to be used to allow the use to set the 
// print enable/disable boost shared memory object used in the printing 
// class and functions that follows.  
// 
// ref: https://stackoverflow.com/questions/6171132/non-blocking-console-input-c
class WatchConsole : private boost::noncopyable {
public:

   WatchConsole();
   ~WatchConsole();

   int Setup();
   bool CheckForInput();

   string GetInput() { return _input; }
   string GetErrorStr() { return _errorStr; }

   void Close();

private:

   int GetUserInput();

   future<int> _fut;
   std::atomic<bool> _quit;

   string _input;
   string _errorStr;

   int _epollFd;
   epoll_event _epollEv;

}; // end WatchConsole
//*******************************************************************


//*******************************************************************
// this class and functions work together to provide a enabled print 
// for debugging or "silent" mode. It uses a boost shared memory 
// object to share enable/disable status to any part of the program 
// without passing an object or a reference.  
// class: SmallIpc includes Writer() that writes an int (flag) to
// the shared memory and then there is a PrintLn() that uses 
// IpcReader() to enable/disable printing. PrintLn() accepts a string 
// to print when the shared memory value is 1 and disables otherwise.
// usage: PrintLn((boost::format{ "loop count %1%" } % i).str());.
// you can use PrintLn() with just a string or boost::format, as shown,
// for more flexible, inline style simplicity
class SmallIpc : private boost::noncopyable {
public:

   SmallIpc();
   ~SmallIpc();

   int Writer(int val);

private:
}; // end SmallIpc

// read the shared memory item
// at this tim, IpcReader() iss not intended for use outside of PrintLn()
// but it could be used in future applications to more than print enabling
int IpcReader();


// print a line if the shared memory item value is 1
// usage: PrintLn((boost::format{ "loop count %1%" } % i).str());
// usage: PrintLn("print something");
void PrintLn(string line);
//*******************************************************************


#endif // end guards