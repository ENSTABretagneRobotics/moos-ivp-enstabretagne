/************************************************************/
/*    FILE: PololuApp.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef PololuApp_HEADER
#define PololuApp_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "Pololu.h"
#include "PololuPinIn.h"
#include "PololuPinOut.h"

class PololuApp : public AppCastingMOOSApp
{
  public:
    PololuApp();
    ~PololuApp();

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // PololuApp functions

  private: // Configuration variables
    std::string m_device_name;

  private: // State variables
    Pololu *m_pololu;
    std::map<std::string,PololuPinIn*> m_map_pinins;
    std::map<std::string,PololuPinOut*> m_map_pinouts;
};

#endif 
