
// header guard
#ifndef PCF8591READER_H
#define PCF8591READER_H

#include <iostream>
#include <iomanip>
#include <string>

#include "Reader.h"
#include "PCF8591.h"
// #include "Util.h"

using namespace std;

// this class must set both 
// _status = ReaderStatus::Complete;
// _status = ReaderStatus::Error;


class PCF8591Reader : public Reader {
public:

   PCF8591Reader();
   virtual ~PCF8591Reader();
   int RunTask() override;
   void SetChannelAndConversion(PCF8591_AI_CHANNEL channel, Scale scale);
   float GetData();

private:

   PCF8591_AI_CHANNEL _channel;
   Scale _scale;
   PCF8591 _pcf8591;
   float _data;

}; // end class 




#endif // end header guard

