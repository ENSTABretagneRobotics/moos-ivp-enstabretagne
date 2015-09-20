/************************************************************/
/*    FILE: Saucisse.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
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
    void quit();

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // Saucisse functions


  private: // Configuration variables
    std::string m_device_name;
    bool m_reset_on_startup;
    bool m_reset_all_on;
    double m_alert_max_temperature;
    int m_status_cameras;
    int m_status_modem;
    int m_status_sonar;
    int m_status_sounder;
    int m_status_gps;
    double m_left_thruster_value;
    double m_right_thruster_value;
    double m_vertical_thruster_value;

  private: // State variables
    Nuc *m_nuc;
    Pololu *m_pololu;
};

#endif 
