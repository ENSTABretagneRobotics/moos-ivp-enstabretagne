/************************************************************/
/*    FILE: Modem.h
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#ifndef Modem_HEADER
#define Modem_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#ifdef _WIN32
  #include "MOOS/libMOOS/Utils/MOOSNTSerialPort.h"
#else
  #include "MOOS/libMOOS/Utils/MOOSLinuxSerialPort.h"
#endif

#include "seanetmsg.h"

#include <fstream>
#include <string.h>
#include <ctime>

#define MESSAGE_MAX_LENGTH 50

class Modem : public AppCastingMOOSApp
{
  public:
    Modem();
    ~Modem();

  protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Modem functions
    void ModemTempoFunction();
    void ListenModemMessages();
    bool receiveMessage(std::string & message, double reception_timeout);
    bool receiveRanging(std::string & message, double reception_timeout);
    bool SendModemConfigurationMessage(const SeaNetMsg & msg){return (m_Port.Write(msg.data().data(), (int)msg.data().size()) == (int)msg.data().size());}

  protected: // Standard AppCastingMOOSApp functions to overload
    bool buildReport();
    void registerVariables();
    void extractMasterName(std::string &);

  private: // Configuration variables
    std::string   m_sRobotName;
    std::string   m_sMasterModemName;
    std::string   m_portName;
    int           m_baudrate_conf;
    int           m_baudrate_comm;
    char          m_modemNodeAddr; //always 85 for modem, setted in Ctor
    double        m_timewarp;

    double beginTime;
    double endTime;



  private: // State variables
    #ifdef _WIN32
      CMOOSNTSerialPort m_Port;
    #else
      CMOOSLinuxSerialPort m_Port;
    #endif
    //Flags for configuration process
    bool          m_bModemConfigurationRequired;
    bool          m_bIsAlive;
    bool          m_bSentCfg;
    bool          m_bGetVersionData;
    bool          m_bGetBBUserData;
    bool          m_bGetFpgaVersionData;
    bool          m_bGetFirstPgrAck;
    bool          m_bGetSecondPgrAck;
    bool          m_bGetThirdPgrAck;
    bool          m_bMtReBootHasBeenSent;
    bool          m_bModemConfiguratonComplete;

    int           m_iModemRoleRequired;
    //Value of timeout applied when sending some configuration commands to the modem
    unsigned int  m_uiTimeoutUS;
    unsigned int  m_uiRngTimeoutUS_param;
    int           m_iTimeBeforeTalking;

    //Configuration for Modem and magnet power supply
    bool m_bIsModemPowered;
    bool m_bIsMagnetPowered;

    int m_iInConfigTime;
    bool m_bInRanging;
    std::string m_sRngStr;
    std::string m_sMsgStr;
    std::string messageReceived;
    std::string m_sLastRangeStr;
    double rangingValue;

    //Thread created for modem configuration
    CMOOSThread   m_serial_thread_conf;
    static bool listen_Modem_config_thread_func(void *pModemObject)
    {
      Modem* pModem = static_cast<Modem*> (pModemObject);
      if (pModem)
      {
        std::cout<<"Configuration thread launched, listenning on serial port for conf process"<<std::endl;
        pModem->ListenModemMessages();
        return true;
      }
      else return false;
    }

    //Thread created for timeouts, probably not the right way to do but it works like a charm
    CMOOSThread   m_serial_thread_tempo;
    static bool listen_Modem_tempo_thread_func(void *pModemObject)
    {
      Modem* pModem = static_cast<Modem*> (pModemObject);
      if (pModem)
      {
        std::cout<<"Timeout configuration thread launched"<<std::endl;
        pModem->ModemTempoFunction();
        return true;
      }
      else return false;
    }
    /*
    * REMINDER :
    * From https://oceanai.mit.edu/svn/moos-ivp-aro/releases/moos-ivp-12.11-beta/MOOS_V10Beta_Dec2112/MOOSCore/Core/libMOOS/Thirdparty/AppCasting/include/MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h
    *   void         reportEvent(const std::string&);
    *   void         reportConfigWarning(const std::string&);
    *   void         reportUnhandledConfigWarning(const std::string&);
    *   void         reportRunWarning(const std::string&);
    *   void         retractRunWarning(const std::string&); //the run warning we want to retract must be exactly the same string as the runWarning
    */



};

#endif
