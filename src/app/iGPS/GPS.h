/************************************************************/
/*    FILE: GPS.h
/*    ORGN: ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef GPS_HEADER
#define GPS_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"

#define KNOTS2MPS 0.5144444444

class GPS : public AppCastingMOOSApp
{
  public:
    GPS();
    ~GPS() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // GPS functions
    bool InitialiseSensor();

    bool GetData();
    bool ParseNMEAString(std::string &s);

    double DMS2DecDeg(double d);

    double Knots2MPS(double s);

  private: // Configuration variables
    unsigned int  m_iterations;
    double        m_timewarp;

    CMOOSGeodesy  m_Geodesy;

    #ifdef _WIN32
      CMOOSNTSerialPort m_Port;
    #else
      CMOOSLinuxSerialPort m_Port;
    #endif
    std::string   m_portName;
    int           m_baudrate;
    bool          m_bPublishRaw;
    bool          m_bIsPowered;


  private: // State variables
    double        m_dGpsTime;
    double        m_dSpeed;
    double        m_dHeading;
    double        m_dYaw;
    double        m_dN;
    double        m_dE;
    double        m_dX;
    double        m_dY;
    double        m_dLongitude;
    double        m_dLatitude;
    double        m_dNbSat;
    double        m_dAltitude;

};

#endif 
