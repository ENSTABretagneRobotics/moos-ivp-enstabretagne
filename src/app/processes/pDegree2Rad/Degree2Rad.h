/************************************************************/
/*    FILE: Degree2Rad.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef Degree2Rad_HEADER
#define Degree2Rad_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class Degree2Rad : public AppCastingMOOSApp
{
  public:
    Degree2Rad();
    ~Degree2Rad() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // Degree2Rad functions


  private: // Configuration variables


  private: // State variables

};

#endif 
