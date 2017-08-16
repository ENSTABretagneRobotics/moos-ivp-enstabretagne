/************************************************************/
/*    FILE: SecuritySurface.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef SecuritySurface_HEADER
#define SecuritySurface_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

using namespace std;
class SecuritySurface : public AppCastingMOOSApp
{
  public:
    SecuritySurface();
    ~SecuritySurface() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // SecuritySurface functions


  private: // Configuration variables


  private: // State variables
    int mLastMessage;
    float mTimeout;
    list< string > variablesNames;
    string publishVariableName;
    string publishValue;
    string currentValue;

};

#endif 
