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
    void findMin(std::vector<double> list, double &min, int &indice, int begin, int end);
    void findMax(std::vector<double> list, double &max, int &indice, int begin, int end);

  private: // Configuration variables
    // FILTER CONFIGURATION
    int m_mean_r, m_mean_theta;
    int m_min_r;
    double m_threshold;
    int m_search_zone;

    // SONAR CONFIGURATION
    double m_sonar_range, m_sonar_gain;
    int m_sonar_nbins;

    bool m_vertical_scan;

  private: // State variables
    std::vector< std::vector<double> > m_scanline_tab;
    std::vector<double> m_bearing_tab;

    // New IMU DATA
    double m_imu_yaw;

    // New SONAR DATA
    bool m_new_scanline;
    std::vector<double> m_new_scanline_data;
    double m_new_bearing;

    // Data Analysis
    double m_max_filtered;
};

#endif 
