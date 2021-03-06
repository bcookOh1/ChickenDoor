
#include "PrintUtils.h"
#include <boost/assert.hpp> 

void vtprint(){
   cout << endl;
} // end vtprint

template <typename T, typename... Types>
void vtprint(T V1, Types... V2){
   cout << V1;
   vtprint(V2...);
} // end vtprint


WatchConsole::WatchConsole() {
   _quit = false;
} // end ctor 

WatchConsole::~WatchConsole() {
} // end dtor 


int WatchConsole::Setup(){

   // create epoll instance
   _epollFd = epoll_create1(0);
   if(_epollFd < 0) {
      _errorStr = "epoll_create1() failed";
      return -1;
   } // end if 

   // associate stdin with _epollEv
   _epollEv.data.ptr = nullptr;
   _epollEv.data.fd = STDIN_FILENO; 
   _epollEv.data.u32 = UINT32_C(0);
   _epollEv.data.u64 = UINT64_C(0);
   _epollEv.events = EPOLLIN;
   if(epoll_ctl(_epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &_epollEv) < 0) {
      _errorStr = "epoll_ctl() failed";
      return -1;
   } // end if 

   return 0;
} // end Setup

// use future/async to watch for input 
bool WatchConsole::CheckForInput() {
   bool ret = false;

   if(_fut.valid() == true) {
      future_status result = _fut.wait_for(0ms);
      if(result == future_status::ready) {

         int stat = _fut.get();
         if(stat < 0){ // error
            BOOST_ASSERT_MSG(false, _errorStr.c_str());
         }
         else if(stat == 0){
            // 0 chars from stdin, user may have hit <cr> only
         }
         else { // stat == 1
            ret = true; // chars available 
         } // end if 
         
         _fut = std::async(std::launch::async, [this]() -> int { return GetUserInput(); });
      } // end if 
   }
   else {
      _fut = std::async(std::launch::async, [this]() -> int { return GetUserInput(); });
   } // end if 

   return ret;
} // end CheckForInput


void WatchConsole::Close() { 
   _quit = true;          // will stop the async() 

   // wait for GetUserInput loop to finish with 75ms 
   _fut.wait_for(75ms);   

   close(_epollFd);       // watching cin events 
   cin.clear();           // restore cin 
} // end quit


int WatchConsole::GetUserInput() {
   string input;

   // input gets the cin data what available
   array<epoll_event, 1> events;
   while(input.size() == 0) {

      // stop checking 
      if(_quit == true) 
         break;
      
      // last parm is 0 for timeout
      int result = epoll_wait(_epollFd, events.data(), events.size(), 0); 
      if(result < 0) {
         _errorStr = "epoll_wait() failed";
         return -1;
      } // end if 

      if(result > 0) {
         getline(cin, input);
         _input = input;
      } // end if 

      this_thread::sleep_for(chrono::milliseconds(50ms));

   } // end while 

   return input.size() > 0 ? 1 : 0;
} // end GetUserInput


SmallIpc::SmallIpc() {
   shared_memory_object::remove( SharedMemoryName.c_str() );
   managed_shared_memory object{ open_or_create, SharedMemoryName.c_str(), 1024 };
   object.construct<int>(SharedMemoryItem.c_str())(1);
} // end ctor 


SmallIpc::~SmallIpc() {
   shared_memory_object::remove( SharedMemoryName.c_str() );
} // end dtor


int SmallIpc::Writer(int val) {
   managed_shared_memory object{ open_only, SharedMemoryName.c_str() };
   pair<int *, size_t> p = object.find<int>(SharedMemoryItem.c_str());

   if(p.first) {
      *p.first = val;
   } // end if 

   return 0;
} // Writer 


int IpcReader() {
   int ret = 0;

   managed_shared_memory object{ open_only, SharedMemoryName.c_str() };
   std::pair<int *, std::size_t> p = object.find<int>(SharedMemoryItem.c_str());

   if(p.first) {
      ret = *p.first;
   } // end if 

   return ret;
} // end IpcReader


void PrintLn(string line) {
   int enable = IpcReader();
   if(enable == 1) {
      cout << line << endl;
   } // end if 
} // end PrintLn

