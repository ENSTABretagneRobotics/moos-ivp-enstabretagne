/************************************************************/
/*    FILE: EchoSounder.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef EchoSounder_HEADER
#define EchoSounder_HEADER

#include "MOOS/libMOOS/App/MOOSApp.h"
#ifdef _WIN32
  #include "MOOS/libMOOS/Utils/MOOSNTSerialPort.h"
#else
  #include "MOOS/libMOOS/Utils/MOOSLinuxSerialPort.h"
#endif

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#define ECHOSOUNDER_MAX_STRING_LENGTH 80

class EchoSounder : public AppCastingMOOSApp
{
  public:
    EchoSounder();
    ~EchoSounder();

  protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload
    bool buildReport();
    void registerVariables();

  protected: // EchoSounder functions
    bool initSerialPort();
    double getRange();
    bool dataIsValid(std::string data);

  private: // Configuration variables


  private: // State variables
    #ifdef _WIN32
      MOOSLinuxSerialPort m_serial_port;
    #else
      CMOOSLinuxSerialPort m_serial_port;
    #endif
    bool m_port_is_initialized;
    std::string m_port_name;
    double m_last_value;
};

#endif
