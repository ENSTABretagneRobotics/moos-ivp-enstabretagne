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
    int bufferLevel;
    double altitude_threshold;
    
    // Topics we subscribe to
    std::string GPS_FIX_SUBSCRIPTION_NAME;
    std::string GPS_SIG_SUBSCRIPTION_NAME;
    std::string KELLER_DEPTH_SUBSCRIPTION_NAME;
    
    // Topics we publish to
    std::string GPS_TRUST_PUBLICATION_NAME;
private: // State variables
    bool gps_quality;
    double gps_fix;
    double altitude; // Current altitude
    std::deque<bool> memory;
    int bufferCounter;
    bool gps_trust;
};

#endif 
