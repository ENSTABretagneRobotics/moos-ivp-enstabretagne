/************************************************************/
/*    FILE: NavigationScreen.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef NavigationScreen_HEADER
#define NavigationScreen_HEADER

#include <map>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class NavigationScreen : public AppCastingMOOSApp
{
  public:
    NavigationScreen();
    ~NavigationScreen() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // NavigationScreen functions


  private: // Configuration variables


  private: // State variables
    std::map<std::string,std::string> m_moosvars;
};

#endif 
