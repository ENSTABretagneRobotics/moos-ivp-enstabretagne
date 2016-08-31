/************************************************************/
/*    FILE: SimplePID.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Thomas Le Mezo and Guilherme Schvarcz Franco
/*    DATE: 2015 - 2016
/************************************************************/

#ifndef SimplePID_HEADER
#define SimplePID_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <string>
#include <iterator>
#include "math.h"
#include "MBUtils.h"
#include "ACTable.h"
#include <math.h>

using namespace std;

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
    string m_angle;

  private: // State variables
    double m_consigne, m_state, m_state_previous, m_state_differential, m_error, m_command;
    double m_staturate;
    double m_time_previous, m_dt;
    double m_I;


};

#endif 
