/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: MapLocalizerParticleFilter.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef MapLocalizerParticleFilter_HEADER
#define MapLocalizerParticleFilter_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "ParticleFilter.h"

class MapLocalizerParticleFilter : public AppCastingMOOSApp {
public:
    MapLocalizerParticleFilter();

    ~MapLocalizerParticleFilter() {
    };

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
    // Altitude where it is considered that GPS
    // data are not reliable
    double altitudeWhereWeIgnoreGPS;
    double gpsEVar;
    double gpsNVar;

private: // State variables
    ParticleFilter pf;

    // The "init" bools are here to make sure we don't take
    // these values into account on startup as long as they havn't 
    // been initialized
    bool lastGPSENInit;
    double lastGPSE;
    double lastGPSN;

    bool lastAltitudeInit;
    double lastAltitude;

    bool lastVelocityInit;
    double lastVelocity;

    bool lastYawInit;
    double lastYaw;
};

#endif
