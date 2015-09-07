/************************************************************/
/*    FILE: Keller.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef Keller_HEADER
#define Keller_HEADER

#include "MOOS/libMOOS/App/MOOSApp.h"
#ifdef _WIN32
  #include "MOOS/libMOOS/Utils/MOOSNTSerialPort.h"
#else
  #include "MOOS/libMOOS/Utils/MOOSLinuxSerialPort.h"
#endif

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "kellermsg.h"


class Keller : public AppCastingMOOSApp
{
  public:
    Keller();
    ~Keller();

  protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload
    bool buildReport();
    void registerVariables();

  protected: // Keller functions
    bool initSerialPort();
    bool initKeller(int maxRetries);
    bool SendKellerMessage(const KellerMsg & msg) {return (m_serial_port.Write(msg.data().data(), (int)msg.data().size()) == (int)msg.data().size());}
    bool ReadPressure();
    bool ReadTemperature();
    bool ResetKellerPressure();
    bool SetZeroKellerPressure();


  private: // Configuration variables
    bool m_port_is_initialized;
    std::string m_port_name;
    double m_lastP_value;
    double m_lastT_value;
    int m_iMmaxRetries;
    bool m_bKellerInitialized;
    bool m_bTemperatureRequested;
    bool m_bKellerPolling;
    double m_bKellerZeroPressure;

    KellerMsg_ReadOutPressureFloatRequest kellerPressureRequest;
    KellerMsg_ReadOutTemperatureFloatRequest kellerTemperatureRequest;
    KellerMsg_ResetPressureRequest kellerResetPressureRequest;
    KellerMsg_SetZeroPressureRequest kellerSetZeroPressureRequest;

  private: // State variables
    #ifdef _WIN32
      MOOSLinuxSerialPort m_serial_port;
    #else
      CMOOSLinuxSerialPort m_serial_port;
    #endif

};

#endif
