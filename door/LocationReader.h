
#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
// #include <boost/bind/bind.hpp>
#include "CommonDef.h"
#include "Reader.h"


using namespace std;
using namespace boost;
// using namespace boost::placeholders,
namespace proptree = boost::property_tree;
namespace bp = boost::process;


// search and replace placeholders in url, note square brackets "[]" are not legal chars in urls
const string STREET_PLACEHOLDER = "[street]"; // the street uses '+' to tie the parts together, i.e. 8243+Forest+Beach+Rd
const string CITY_PLACEHOLDER = "[city]";
const string STATE_PLACEHOLDER = "[state]";
const string ZIP_CODE_PLACEHOLDER = "[zip]";

const string RESPONSE_FILE = "coords.json";

// ref for timeouts: https://stackoverflow.com/questions/42873285/curl-retry-mechanism
// https://geocoding.geo.census.gov/geocoder/locations/address?street=8243+Forest+Beach+Rd&city=Watervliet&state=Michigan&zip=49098&benchmark=2020&format=json
const string CURL_LOCATION_CMD = "curl -s -o " + RESPONSE_FILE
   + " \"https://geocoding.geo.census.gov/geocoder/locations/address?street=[street]&city=[city]&state=[state]&zip=[zip]&benchmark=2020&format=json\"";


class LocationReader : public Reader {
public:

   LocationReader(const Address address);
   virtual ~LocationReader();

   int RunTask() override;
   Coordinates GetData() { return _coords; }

private:

   int ReplacePlaceholders();
   int ParseResponse();

   Address _address;
   Coordinates _coords;
   string _curlCmd;

   unique_ptr<bp::child> _child;

}; // end class


/* a good response for my address, see first "addressMatches"
{
   "result": {
      "input": {  
         "address": {
            "zip": "49098",
            "city": "Watervliet",
            "street": "8243 Forest Beach Rd",
            "state": "Michigan"
         },
         "benchmark": {
            "isDefault": false,
            "benchmarkDescription": "Public Address Ranges - Census 2020 Benchmark",
            "id": "2020",
            "benchmarkName":"Public_AR_Census2020"
         }
      },
      "addressMatches": [{
         "tigerLine": {
            "side":"L",
            "tigerLineId":"119998567"
         },
         "coordinates": {
            "x":-86.2584756050756,
            "y":42.200443994081276
         },
         "addressComponents": { 
            "zip":"49098",
            "streetName":"FOREST BEACH",
            "preType":"",
            "city":"WATERVLIET",
            "preDirection":"",
            "suffixDirection":"",
            "fromAddress":"8201",
            "state":"MI",
            "suffixType":"RD",
            "toAddress":"8299",
            "suffixQualifier":"",
            "preQualifier":""
         },
            "matchedAddress":"8243 FOREST BEACH RD, WATERVLIET, MI, 49098"
      }]
   }
}
// path to the data 
result.addressMatches[0].x // longitude
result.addressMatches[0].y // latitude

*/
