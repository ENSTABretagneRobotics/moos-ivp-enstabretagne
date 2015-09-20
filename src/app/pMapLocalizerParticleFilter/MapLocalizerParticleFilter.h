/************************************************************/
/*    AUTH: Jeremy NICOLA                                   */
/*    ORGN: Toutatis AUVs - ENSTA Bretagne                  */
/*    FILE: MapLocalizerParticleFilter.h                    */
/*    DATE: 2015                                            */
/************************************************************/

#ifndef MapLocalizerParticleFilter_HEADER
#define MapLocalizerParticleFilter_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "ParticleFilter.h"

class MapLocalizerParticleFilter : public AppCastingMOOSApp {
public:
    MapLocalizerParticleFilter();
    ~MapLocalizerParticleFilter() {};
    void quit() {};

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
    std::string mapPath;
    double speedVar;
    double headingVar;
    double altitudeVar;
    double gpsEVar;
    double gpsNVar;
    double beamAngleVar;
    double beamRangeVar;
    
    // After how many scanlines do we resample?
    // a good idea is to wait for at leat a full turn
    int resampleEvery;
    
    // MULTINOMIAL,LOW_VARIANCE,RESIDUAL
    int resamplingMethod;    

private: // State variables
    ParticleFilter pf;

    // The "init" bools are here to make sure we don't take
    // these values into account on startup as long as they havn't 
    // been initialized
    bool filter_easting_initialized;
    bool filter_northing_initialized;
    bool gps_trust;
    bool mission_started;
    
    double lastGPSE;
    double lastGPSN;

    double lastAltitude;

    double lastVelocity;

    double lastYaw;
    
    double beamAngle;
    double beamRange;
};

#endif
