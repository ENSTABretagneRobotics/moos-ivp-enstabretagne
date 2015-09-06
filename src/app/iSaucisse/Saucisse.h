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


  private: // Configuration variables
    std::string m_device_name;

  private: // State variables
    Pololu *m_pololu;
};

#endif 
