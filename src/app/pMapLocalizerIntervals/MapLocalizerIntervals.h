/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: MapLocalizerIntervals.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef MapLocalizerIntervals_HEADER
#define MapLocalizerIntervals_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "interval-filter-localization/maplocalizer.h"

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
     int qSonar;
     std::string mapPath;
     double speedNoise;
     double headingNoise;
     double gps_noise;
     int bufferSize;
 private: // State variables
     bool filter_easting_initialized;
     bool filter_northing_initialized;
     bool gps_trust;
     double gps_easting;
     double gps_northing;
     double speed;
     double theta;
     double beamRange;
     double beamAngle;
     MapLocalizer localizer;
};

#endif 
