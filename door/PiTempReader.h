
// header guard
#ifndef PITEMPREADER_H
#define PITEMPREADER_H

#include <iostream>
#include <iomanip>
#include <string>

#include "Reader.h"
#include "Util.h"

using namespace std;

class PiTempReader : public Reader {
public:

   PiTempReader();
   virtual ~PiTempReader();

   int RunTask() override;
   string GetData();


private:

   string _temperature;


}; // end class


#endif // end header guard
