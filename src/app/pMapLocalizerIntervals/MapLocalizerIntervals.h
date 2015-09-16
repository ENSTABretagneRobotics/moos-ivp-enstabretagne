/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: MapLocalizerIntervals.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef MapLocalizerIntervals_HEADER
#define MapLocalizerIntervals_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class MapLocalizerIntervals : public AppCastingMOOSApp
{
 public:
   MapLocalizerIntervals();
   ~MapLocalizerIntervals() {};

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();

 private: // Configuration variables

 private: // State variables
};

#endif 
