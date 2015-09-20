/************************************************************/
/*    FILE: SimplePID.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Thomas Le Mezo
/*    DATE: 2015
/************************************************************/

#ifndef SimplePID_HEADER
#define SimplePID_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <string>

class SimplePID : public AppCastingMOOSApp
{
  public:
    SimplePID();
    ~SimplePID() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // SimplePID functions


  private: // Configuration variables
    double m_kp, m_ki, m_kd, m_kw;
    std::string m_moosvar_state; 
    std::string m_moosvar_state_diffferential;
    std::string m_moosvar_consigne;
    std::string m_moosvar_command;
    std::string m_moosvar_saturation;
    
    bool m_differential_input;
    bool m_angle_degree;

  private: // State variables
    double m_consigne, m_state, m_state_previous, m_state_differential, m_error, m_command;
    double m_staturate;
    double m_time_previous, m_dt;
    double m_I;


};

#endif 
