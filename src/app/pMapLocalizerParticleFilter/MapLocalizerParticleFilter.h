/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: MapLocalizerParticleFilter.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef MapLocalizerParticleFilter_HEADER
#define MapLocalizerParticleFilter_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class MapLocalizerParticleFilter : public AppCastingMOOSApp
{
 public:
   MapLocalizerParticleFilter();
   ~MapLocalizerParticleFilter() {};

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
