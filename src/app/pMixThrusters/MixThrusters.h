/************************************************************/
/*    FILE: MixThrusters.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef MixThrusters_HEADER
#define MixThrusters_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class MixThrusters : public AppCastingMOOSApp
{
  public:
    MixThrusters();
    ~MixThrusters() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // MixThrusters functions


  private: // Configuration variables


  private: // State variables
    

};

#endif 
