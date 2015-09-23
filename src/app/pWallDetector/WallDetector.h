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
#include "SonarWall.h"

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

  private: // Configuration variables
    SonarWall sonar_micron;
    SonarWall sonar_miniking;
    SonarWall sonar;

    double m_imu_yaw;
    
  private: // State variables
    
};

#endif 
