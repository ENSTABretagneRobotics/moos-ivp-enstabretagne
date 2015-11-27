/************************************************************/
/*    FILE: Saucisse.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef Saucisse_HEADER
#define Saucisse_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "Pololu.h"
#include "Nuc.h"

class Saucisse : public AppCastingMOOSApp
{
  public:
    Saucisse();
    ~Saucisse();

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // Saucisse functions
    std::string sensorStatusInText(int status);

  private: // Configuration variables
    std::string m_device_name;
    bool m_autoset_on_startup, m_default_value_on_startup;
    bool m_autoset_on_quit, m_default_value_on_quit;

  private: // State variables
    Nuc *m_nuc;
    Pololu *m_pololu;
    int m_status_cameras;
    int m_status_sounder;
    int m_status_sonar;
    int m_status_modem;
    //int m_status_micron;
    //int m_status_miniking;
    int m_status_gps;
    double m_left_thruster_value;
    double m_right_thruster_value;
    double m_vertical_thruster_value;
};

#endif 
