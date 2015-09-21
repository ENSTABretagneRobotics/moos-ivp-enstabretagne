/************************************************************/
/*    FILE: OpiDetection.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef OpiDetection_HEADER
#define OpiDetection_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class OpiDetection : public AppCastingMOOSApp
{
  public:
    OpiDetection();
    ~OpiDetection() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // OpiDetection functions


  private: // Configuration variables


  private: // State variables

};

#endif 
