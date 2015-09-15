/************************************************************/
/*    FILE: WallDetector.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef WallDetector_HEADER
#define WallDetector_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <vector>

using namespace std;

class WallDetector : public AppCastingMOOSApp
{

  public:
    WallDetector();
    ~WallDetector() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // WallDetector functions
    double MOOSGrad2Rad(double angle);

  private: // Configuration variables
    // FILTER CONFIGURATION
    int mean_r, mean_theta;
    int min_r;
    double threshold;

    // SONAR CONFIGURATION
    double sonar_range, sonar_gain;
    int sonar_nbins;

  private: // State variables
    vector< vector<double> > scanline_tab;
    vector<double> bearing_tab;

    // New IMU DATA
    double imu_yaw;

    // New SONAR DATA
    bool new_scanline;
    vector<double> new_scanline_data;
    double new_bearing;
};

#endif 
