
// ref: https://www.youtube.com/watch?v=UcYankkrIpw
// desc: simple class to pipe commands to gnuplot

#ifndef GNUPLOT_HPP
#define GNUPLOT_HPP

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

class GnuPlot {
public:
   GnuPlot(){
      _gnuplotpipe = popen("gnuplot", "w");
      if(!_gnuplotpipe)
         cerr << "gnuplot not found" << endl;
   } // end ctor 
   
   ~GnuPlot(){
      fprintf(_gnuplotpipe,"exit\n");
      pclose(_gnuplotpipe);
   } // end dtor 
   
   void operator()(const string &command){
      fprintf(_gnuplotpipe,"%s\n", command.c_str());
      fflush(_gnuplotpipe);
   } // end operator()
   
protected:
   FILE *_gnuplotpipe;
}; // end class

#endif
