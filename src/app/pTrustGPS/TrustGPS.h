/************************************************************/
/*    NAME: Jeremy NICOLA                                   */
/*    ORGN: MIT                                             */
/*    FILE: TrustGPS.h                                      */
/*    DATE: 2015                                            */
/************************************************************/

#ifndef TrustGPS_HEADER
#define TrustGPS_HEADER

#include <deque>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class TrustGPS : public AppCastingMOOSApp {
public:
    TrustGPS();
    ~TrustGPS() {};

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
    int paranoiaLevel;
    double altitude_threshold;
private: // State variables
    bool gps_quality;
    int gps_fix;
    double altitude; // Current altitude
    std::deque<bool> memory;
    int paranoiaCounter;
    bool gps_trust;
};

#endif 
