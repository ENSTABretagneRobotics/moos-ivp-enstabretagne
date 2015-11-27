/************************************************************/
/*    FILE: LatLon2LocalGrid.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef LatLon2LocalGrid_HEADER
#define LatLon2LocalGrid_HEADER

#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class LatLon2LocalGrid : public AppCastingMOOSApp {
public:
    LatLon2LocalGrid();

    ~LatLon2LocalGrid() {
    };

protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

protected: // LatLon2LocalGrid functions


private: // Configuration variables
    double lat_origin;
    double long_origin;
    double m_custom_shift_x;
    double m_custom_shift_y;

    // Topics we subscribe to
    std::string LAT_SUBSCRIPTION_NAME;
    std::string LON_SUBSCRIPTION_NAME;
    
    // Topics we publish to
    std::string NORTHING_PUBLICATION_NAME;
    std::string EASTING_PUBLICATION_NAME;

    CMOOSGeodesy geodesy;
private: // State variables
    double currentLat;
    double currentLon;
    double currentEasting;
    double currentNorthing;
};

#endif 
