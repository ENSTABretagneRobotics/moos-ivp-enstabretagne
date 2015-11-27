/************************************************************/
/*    AUTH: Jeremy NICOLA                                   */
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne                  */
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
    
    // Subscriptions
    std::string RESET_SUBSCRIPTION_NAME;
    std::string WALL_DETECTOR_SUBSCRIPTION_NAME;
    std::string GPS_E_SUBSCRIPTION_NAME;
    std::string GPS_N_SUBSCRIPTION_NAME;
    std::string KELLER_DEPTH_SUBSCRIPTION_NAME;
    std::string SPEED_ESTIM_LOCAL_SUBSCRIPTION_NAME;
    std::string IMU_YAW_SUBSCRIPTION_NAME;
    std::string GPS_TRUST_SUBSCRIPTION_NAME;
    
    // Publications
    std::string POS_X_PUBLICATION_NAME;
    std::string POS_Y_PUBLICATION_NAME;
    std::string POS_COV_XX_PUBLICATION_NAME;
    std::string POS_COV_XY_PUBLICATION_NAME;
    std::string POS_COV_YY_PUBLICATION_NAME;
    
    std::string POS_X_WEIGHTED_PUBLICATION_NAME;
    std::string POS_Y_WEIGHTED_PUBLICATION_NAME;
    std::string POS_COV_XX_WEIGHTED_PUBLICATION_NAME;
    std::string POS_COV_XY_WEIGHTED_PUBLICATION_NAME;
    std::string POS_COV_YY_WEIGHTED_PUBLICATION_NAME;
private: // State variables
    ParticleFilter pf;

    // The "init" bools are here to make sure we don't take
    // these values into account on startup as long as they havn't 
    // been initialized
    bool filter_easting_initialized;
    bool filter_northing_initialized;
    bool gps_trust;
    
    double lastGPSE;
    double lastGPSN;

    double lastAltitude;

    double lastVelocity;

    double lastYaw;
    
    double beamAngle;
    double beamRange;
    bool receivedBeam;
};

#endif
