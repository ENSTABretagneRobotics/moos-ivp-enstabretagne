/************************************************************/
/*    FILE: Rad2Degree.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef Rad2Degree_HEADER
#define Rad2Degree_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class Rad2Degree : public AppCastingMOOSApp
{
  public:
    Rad2Degree();
    ~Rad2Degree() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // Rad2Degree functions


  private: // Configuration variables


  private: // State variables

};

#endif 
