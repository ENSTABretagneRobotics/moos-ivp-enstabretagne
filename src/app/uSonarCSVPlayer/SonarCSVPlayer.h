/************************************************************/
/*    FILE: SonarCSVPlayer.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#ifndef SonarCSVPlayer_HEADER
#define SonarCSVPlayer_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class SonarCSVPlayer : public AppCastingMOOSApp
{
  public:
    SonarCSVPlayer();
    ~SonarCSVPlayer() {};

  protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload
    bool buildReport();
    void registerVariables();

  protected: // SonarCSVPlayer functions


  private: // Configuration variables


  private: // State variables
    FILE* m_FileHandler = NULL;
    std::string m_sFilename;

};

#endif
