/************************************************************/
/*    FILE: Modem.cpp
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Modem.h"
#include "iModemUtils.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Modem::Modem()
{
  m_portName = "/dev/ttyUSB0";
  m_baudrate_conf = 57600;
  m_baudrate_comm =  9600;
  m_modemNodeAddr = 85; //Always 85 for Micron Data Modems
  m_timewarp   = 1;

  m_bModemConfigurationRequired = false;
  m_bIsAlive = false;
  m_bSentCfg = false;
  m_bGetVersionData = false;
  m_bGetBBUserData = false;
  m_bGetFpgaVersionData = false;
  m_bGetFirstPgrAck = false;
  m_bGetSecondPgrAck = false;
  m_bGetThirdPgrAck = false;
  m_bMtReBootHasBeenSent = false;
  m_bModemConfiguratonComplete = false;

  m_iModemRoleRequired = 0; //0 = master, 1 = slave
  m_uiTimeoutUS = 0;
  m_uiRngTimeoutUS_param = 0;
  m_iTimeBeforeTalking = 30;
  m_iInConfigTime = 0;
  m_bInRanging = false;
  m_sRngStr="";
  m_sMsgStr="";
  messageReceived = "";
  rangingValue = 7777.777;
  m_sMasterModemName="";

  m_bIsModemPowered = false;
  m_bIsMagnetPowered = false;

  //Init threads
  if (!m_serial_thread_conf.Initialise(listen_Modem_config_thread_func, (void*)this))
    reportRunWarning("iModem: Modem thread listen initialization error...\n");
  if (!m_serial_thread_tempo.Initialise(listen_Modem_tempo_thread_func, (void*)this))
    reportRunWarning("iModem: Modem thread tempo initialization error...\n");
}
Modem::~Modem()
{
  reportEvent("iModem: stopping threads.\n");
  m_serial_thread_conf.Stop();
  m_serial_thread_tempo.Stop();
  //Stop Magnet and modem alimentation
  Notify("POWER_MODEM",0.0);
  Notify("POWER_MODEM_EA",0.0);
  reportEvent("iModem: finished.\n");
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Modem::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

    #if 0 // Keep these around just for template
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString();
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif

    if ( key == "MODEM_CONFIGURATION_REQUIRED" && msg.IsString() )
    {

      string robotRole;
      //search for an AUVx=master to store it in m_sMasterModemName string in order to know who is the master
      m_sMasterModemName = msg.GetString();
      // reportEvent("iModem: extracting masters name from ["+m_sMasterModemName+"].\n");
      extractMasterName(m_sMasterModemName);
      reportEvent("iModem: extracted masters name: ["+m_sMasterModemName+"].\n");
      //if we found a string m_sRobotName=role, configure us as role
      if(!MOOSValFromString(robotRole, msg.GetString(), m_sRobotName))
        reportRunWarning(msg.GetKey() + ": Unable to find my role in MODEM_CONFIGURATION_REQUIRED");
      else
      {
        m_iModemRoleRequired = -1;
        if (strcmp("master",robotRole.c_str()))
          m_iModemRoleRequired = 1;
        if (strcmp("slave",robotRole.c_str()))
          m_iModemRoleRequired = 0;
        if (m_iModemRoleRequired != -1)
        {
          m_bModemConfigurationRequired = true;
          m_iInConfigTime = 1;
          reportEvent("iModem: New Mail asking new modem configuration as " + msg.GetString() + "\n");
          beginTime = MOOSTime();
        }
        else
          reportRunWarning("iModem: ERROR: New Mail asking new modem configuration as ["+msg.GetString()+"], ASKED CONFIGURATION UNRECOGNIZED (case sensitive)\n");
      }
    }
    else if ( key == "MODEM_SEND_RANGE" && msg.IsString() )
    {
      string messageToSent;
      //try to fiund a string m_sRobotName=role
      if(!MOOSValFromString(messageToSent, msg.GetString(), m_sRobotName))
        reportRunWarning("iModem: Unable to find my role in MODEM_SEND_RANGE variable");
      else
      {
        retractRunWarning("iModem: Unable to find my role in MODEM_SEND_RANGE variable");
        if (!m_bModemConfigurationRequired && m_Port.GetBaudRate() == 9600)
        {
          char buffer[100] = {0};
          int bs=0;
          if (rangingValue == 7777.777)
            bs = sprintf(buffer,"d%s%s=TMO",m_sRobotName.c_str(),m_sMasterModemName.c_str());
          else
            bs = sprintf(buffer,"d%s%s=%3.3fm",m_sRobotName.c_str(),m_sMasterModemName.c_str(),rangingValue);
          m_Port.Write(buffer, bs);
          reportEvent("iModem: Range message ["+string(buffer)+"] sent.\n");
          // Notify("MODEM_RANGEMSG_SENT", buffer);
          retractRunWarning("iModem: Cannot send range message, modem could be in a configuration step or serial port baddly configured\n");
        }
        else
          reportRunWarning("iModem: Cannot send range message, modem could be in a configuration step or serial port baddly configured\n");
      }
    }
    else if ( key == "MODEM_SEND_MESSAGE" && msg.IsString() )
    {
      string messageToSent;
      //if we found a string m_sRobotName=role, configure us as role
      if(!MOOSValFromString(messageToSent, msg.GetString(), m_sRobotName))
        reportRunWarning("iModem: Unable to find my role in MODEM_SEND_MESSAGE variable");
      else
      {
        retractRunWarning("iModem: Unable to find my role in MODEM_SEND_MESSAGE variable");
        if (!m_bModemConfigurationRequired && m_Port.GetBaudRate() == 9600)
        {
          m_Port.Write(messageToSent.c_str(), messageToSent.size());
          reportEvent("iModem: Message ["+messageToSent+"] sent.\n");
          // char buffer[100] = {0};
          // sprintf(buffer,"%s=%s",m_sRobotName.c_str(),messageToSent.c_str());
          // Notify("MODEM_MESSAGE_SENT", buffer);
          retractRunWarning("iModem: Cannot send message, modem could be in a configuration step or serial port baddly configured\n");
        }
        else
          reportRunWarning("iModem: Cannot send message, modem could be in a configuration step or serial port baddly configured\n");
      }
    }
    // else if ( key == "MODEM_TIME_BEFORE_TALKING")
    // {
    //   m_iTimeBeforeTalking = msg.GetDouble();
    // }
    else if(key == "POWERED_MODEM")
    {
        m_bIsModemPowered = (msg.GetDouble())?true:false;
        if (m_iInConfigTime == 2)
          m_iInConfigTime = 3;
        else if (m_iInConfigTime == 6)
          m_iInConfigTime = 7;
        else if (m_iInConfigTime == 12)
          m_iInConfigTime = 13;
        else if (m_iInConfigTime == 14)
          m_iInConfigTime = 15;
        else
          MOOSTrace("ModemManager: Error receiving POWERED_MODEM acknowledgement, unhandled case\n");
    }
    else if  (key == "POWERED_MODEM_EA")
    {
      m_bIsMagnetPowered = (msg.GetDouble())?true:false;
      if (m_iInConfigTime == 4)
        m_iInConfigTime = 5;
      else if (m_iInConfigTime == 9)
        m_iInConfigTime = 10;
      else
        MOOSTrace("ModemManager: Error receiving POWERED_MODEM_EA acknowledgement, unhandled case\n");
    }
    else if(key == "MODEM_STOP_ALIM")
    {
      Notify("POWER_MODEM",0.0);
      Notify("POWER_MODEM_EA",0.0);
    }
    else if(key == "MODEM_SEND_RNG")
    {
      string messageToSent;
      //if we found a string m_sRobotName=role, configure us as role
      if(!MOOSValFromString(messageToSent, msg.GetString(), m_sRobotName))
        reportRunWarning(msg.GetKey() + ": Unable to find my role in MODEM_SEND_RNG variable");
      else
      {
        //Start timeout timer for rng
        string ranging("rng\n\0");
        if (!m_bModemConfigurationRequired && m_Port.GetBaudRate() == 9600)
        {
          m_Port.Write(ranging.c_str(), ranging.size());
          reportEvent("iModem: Message ["+ranging+"] sent.\n");
          // char buffer[100] = {0};
          // sprintf(buffer,"%s=%i",m_sRobotName.c_str(),true);
          // Notify("MODEM_RNG_SENT",buffer);
          m_bInRanging = true;
          reportEvent("iModem: rng sent, ranging mode activated.\n");
          retractRunWarning("iModem: Cannot send message, modem could be in a configuration step or serial port baddly configured\n");
        }
        else
          reportRunWarning("iModem: Cannot send message, modem could be in a configuration step or serial port baddly configured\n");
      }
    }
    else if(key == "MODEM_ACK_RANGE")
    {
      string messageToSent;
      //if we found a string m_sRobotName=role, configure us as role
      if(!MOOSValFromString(messageToSent, msg.GetString(), m_sRobotName))
        reportRunWarning(msg.GetKey() + ": Unable to find my role in MODEM_ACK_RANGE variable");
      else
      {
        if (!m_bModemConfigurationRequired && m_Port.GetBaudRate() == 9600)
        {
          m_Port.Write(m_sLastRangeStr.c_str(), m_sLastRangeStr.size());
          reportEvent("iModem: Message ["+m_sLastRangeStr+"] sent.\n");
          // char buffer[100] = {0};
          // sprintf(buffer,"%s=%i",m_sRobotName.c_str(),true);
          // Notify("MODEM_RNG_ACK_SENT",buffer);
          reportEvent("iModem: rng ack sent.\n");
          retractRunWarning("iModem: Cannot send MODEM_ACK_RANGE, modem could be in a configuration step or serial port baddly configured\n");
          m_sLastRangeStr="unknown";
        }
        else
          reportRunWarning("iModem: Cannot send MODEM_ACK_RANGE, modem could be in a configuration step or serial port baddly configured\n");
      }
    }
    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }
  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Modem::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Modem::Iterate()
{
  AppCastingMOOSApp::Iterate();
  char buffer[100] = {0};
  double temps_secs;
  string message;
  //First part of Iterate : when modem is in config mode
  if(m_bModemConfigurationRequired)
  {
    m_sRngStr="";
    m_sMsgStr="";
    messageReceived="";
    m_sLastRangeStr="";
    //if not, configure serial port in config mode (baudrate = 57600)
    if (m_Port.GetBaudRate() == 9600)
    {
      //Strange code to put an int to a string...
      string tt;
      string ttt("BAUDRATE=");
      char numstr[21] = {0}; // enough to hold all numbers up to 64-bits
      sprintf(numstr, "%d", m_baudrate_conf);
      tt = ttt + numstr;
      STRING_LIST params;
      params.push_back(tt);
      m_Port.Close();
      reportEvent("iModem: Configuring "+m_portName+" serial port at baud rate "+numstr+"\n");
      bool portOpened = m_Port.Configure(params);
      // reportEvent("iModem: Serial port configured\n");
      //m_Port.SetTermCharacter('\n');
      m_Port.Flush();
      MOOSPause(1000);
      m_timewarp = GetMOOSTimeWarp();
    }
    //Configuration Process
    switch (m_iInConfigTime) {
      case 0:
        MOOSTrace("ModemManager: Not in Config Mode, nothing to do\n");
      break;
      case 1:
        reportEvent("iModem: Configuration Process started\n");
        // sprintf(buffer,"%s=%f",m_sRobotName.c_str(),MOOSTime());
        // Notify("MODEM_START_CONFIG_TIME", buffer);
        // 2) notify to power down modem
        // MOOSTrace("ModemManager: Asking modem powering down\n");
        Notify("POWER_MODEM",0.0);
        m_iInConfigTime=2;
      break;
      case 2:
        MOOSTrace("ModemManager: Modem powering down asked, waiting aknowledgement to power up the magnet\n");
      break;
      case 3:
        MOOSTrace("ModemManager: Modem powered down aknowledged\n");
        //Re launch configuration thread if not running
        if (!m_serial_thread_conf.IsThreadRunning())
          m_serial_thread_conf.Start();
        // 3) notify to power up magnet
        MOOSTrace("ModemManager: Asking magnet powering up\n");
        Notify("POWER_MODEM_EA",1.0);
        m_iInConfigTime=4;
      break;
      case 4:
        MOOSTrace("ModemManager: Magnet powering up asked, waiting aknowledgement to power up the modem\n");
      break;
      case 5:
        MOOSTrace("ModemManager: Modem powered off and magnet powered up acknowledged, can power up the modem \n");
        // 4) notify to power up modem
        Notify("POWER_MODEM",1.0);
        //We are now waiting iModem to say that modem is alive in order to power down the magnet
        m_iInConfigTime=6;
      break;
      case 6:
        MOOSTrace("ModemManager: Modem powering up asked, waiting aknowledgement\n");
      break;
      case 7:
        // ModemManager asked for modem powering down, magnet powering up
        MOOSTrace("ModemManager: Modem not (yet) alive\n");
      break;
      case 8:
        MOOSTrace("ModemManager: Modem is alive, asking to power down the magnet\n");
        // 5) iModem has published a MODEM_IS_ALIVE message, notify to power down magnet
        Notify("POWER_MODEM_EA",0.0);
        m_iInConfigTime=9;
      break;
      case 9:
        MOOSTrace("ModemManager: Magnet power off asked\n");
      break;
      case 10:
        MOOSTrace("ModemManager: Magnet powered off acknowledged, waiting modem configuration to be complete\n");
      break;
      case 11:
        // 6) when iModem publish an MODEM_CONFIGURATION_COMPLETE message,
        //     notify to power down modem,
        MOOSTrace("ModemManager: Modem Configuration complete, asking iLabjack modem powering down\n");
        Notify("POWER_MODEM",0.0);
        m_iInConfigTime=12;
      break;
      case 12:
        MOOSTrace("ModemManager: modem powering down asked\n");
      break;
      case 13:
        MOOSTrace("ModemManager: Modem powered off acknowledged, can power up on the modem\n");
        // MOOSPause(500);
        Notify("POWER_MODEM",1.0);
        m_iInConfigTime=14;
      break;
      case 14:
        MOOSTrace("ModemManager: modem powering up asked\n");
      break;
      case 15:
        MOOSTrace("ModemManager: Modem powered on after configuration acknowledged, configuration process complete\n");
        endTime = MOOSTime();
        // sprintf(buffer,"%s=%f",m_sRobotName.c_str(),MOOSTime());
        // Notify("MODEM_END_CONFIG_TIME", buffer);
        // sprintf(buffer,"%s=%f",m_sRobotName.c_str(),endTime-beginTime);
        // Notify("MODEM_CONFIG_TIME_SECS", buffer);
        // reportEvent("iModem: Configuration Process ended\n");
        sprintf (buffer, "iModem: Time elapsed during configuration process : %f seconds",endTime-beginTime);
        reportEvent(buffer);
        m_bModemConfigurationRequired = false;
        m_bIsAlive = false;
        m_bSentCfg = false;
        m_bGetVersionData = false;
        m_bGetBBUserData = false;
        m_bGetFpgaVersionData = false;
        m_bGetFirstPgrAck = false;
        m_bGetSecondPgrAck = false;
        m_bGetThirdPgrAck = false;
        m_bModemConfiguratonComplete = false;
        m_bMtReBootHasBeenSent = false;
        break;
      default:
        MOOSTrace("ModemManager: Lost in config mode\n");
      break;
    }
  }
  else
  {
    //stop configuration/tempo thread if running
    if (m_serial_thread_conf.IsThreadRunning())
      m_serial_thread_conf.Stop();
    if (m_serial_thread_tempo.IsThreadRunning())
      m_serial_thread_tempo.Stop();

    //Configure serial port in comm mode (baudrate = 9600)
    if (m_Port.GetBaudRate() == 57600)
    {
      //Strange code to put an int to a string...
      string tt;
      string ttt("BAUDRATE=");
      char numstr[21]={0}; // enough to hold all numbers up to 64-bits
      sprintf(numstr, "%d", m_baudrate_comm);
      tt = ttt + numstr;
      STRING_LIST params;
      params.push_back(tt);
      m_Port.Close();
      reportEvent("iModem: Configuring "+m_portName+" serial port at baud rate "+numstr+"\n");
      bool portOpened = m_Port.Configure(params);
      // reportEvent("iModem: Serial port configured\n");
      //m_Port.SetTermCharacter('\n');
      m_Port.Flush();
      MOOSPause(1000);
      m_timewarp = GetMOOSTimeWarp();
    }
    if(m_bInRanging)
    {
      m_sMsgStr="";
      m_sLastRangeStr="";
      messageReceived="";
      if(receiveMessage(message, 1))
      {
        reportEvent("iModem: Ranging mode, receiving ["+message+"]\n");
        // sprintf(buffer,"%s=%s",m_sRobotName.c_str(),message.c_str());
        // Notify("MODEM_RANGING_BUFFER_RECEIVED", buffer);
        // Notify("MODEM_RANGING_MESSAGE_RECEIVED", message.c_str());
        m_sRngStr += message;
        // stripUnicode(m_sRngStr);
        // stripCRLF7F(m_sRngStr);
        stripCRLF(m_sRngStr);
        if (m_sRngStr.compare(0,8,"RangeTMO") == 0)
        {
          rangingValue = 7777.777;
          sprintf(buffer,"%s=%s",m_sRobotName.c_str(),m_sRngStr.c_str());
          Notify("MODEM_RANGING_TIMEOUT", buffer);
          reportEvent("iModem: Ranging Timeout (from modem)\n");
          // MOOSTrace("iModem: Ranging Timeout******************************************************\n");
          m_sRngStr="";
          m_bInRanging = false;
        }
        else if (m_sRngStr.compare(0,6,"Range=") == 0 && m_sRngStr.size() >= 10)
        {
          reportEvent("iModem: m_sRngStr.compare(0,6,\"Range=\") == 0 && m_sRngStr.size() >= 10");
          unsigned int foundM = m_sRngStr.find_last_of('m');
          string meters = m_sRngStr.substr (0,foundM);
          // char fundMStr[5]={0};
          // sprintf(fundMStr,"%d",foundM);
          // reportEvent("iModem:  METERS extracted = "+meters+", foundM = "+fundMStr+"\n");

          if(!MOOSValFromString(rangingValue, meters, "Range"))
            reportRunWarning(meters + ": Unable to find correct string");
          else
          {
            /************ rangingValue is a double that contain the value returned by ranging function ******************/
            // printf("ranging return [%s]******************************************************\n",meters.c_str());
            // sprintf(buffer,"%s=%s",m_sRobotName.c_str(),m_sRngStr.c_str());
            // Notify("MODEM_RANGING_RECEIVED", m_sRngStr);
            sprintf(buffer,"%s=%f",m_sRobotName.c_str(),rangingValue);
            Notify("MODEM_RANGING_VALUE", buffer);
            reportEvent("iModem: Ranging received = "+m_sRngStr+"\n");
            // MOOSTrace("iModem: Ranging received = [%s]\n", m_sRngStr.c_str());
            m_sRngStr="";
            m_bInRanging = false;
          }
        }
        else
        {
        reportEvent("iModem: Ranging mode, unable to find Range= or RangeTMO in ["+m_sRngStr+"]\n");
        // sprintf(buffer,"%s=%s",m_sRobotName.c_str(),m_sRngStr.c_str());
        // Notify("MODEM_RNG_MSG_PARSE_ERROR", buffer);
        }
      }
    }
    else if (receiveMessage(messageReceived, 1))
    {
      m_sRngStr="";
      reportEvent("iModem: Receiving ["+messageReceived+"]\n");
        // MOOSTrace("iModem: Receiving [%s]\n", message.c_str());
      m_sMsgStr+=messageReceived;
      stripUnicode(m_sMsgStr);
      stripCRLF7F(m_sMsgStr);
        //From http://cboard.cprogramming.com/c-programming/115509-validation-string-format.html
        // Elegant solution to check if a string 's' is formated like "&x,&y,&z" with x, y and z integers
        // char a[2], b[20], c[2], d[2], e[20], f[2], g[2], h[20];
        // char s[] = "&123456,&1007,&9";
        // if (sscanf(s, "%[&]%[0-9]%[,]%[&]%[0-9]%[,]%[&]%[0-9]", a, b, c, d, e, f, g, h) == 8)
        //     printf("okay\n");
        bool distanceFounded = false;
        string msgToParse = m_sMsgStr;
        uint tailleMin = 13; //"dAUVxAUVy=z.zm"
        // uint tailleMax = 18; //"dAUVxAUVy=zzz.zzzm"//pour l'instant ne sert pas
        if (msgToParse.size() >= tailleMin)
        {
          char sd[2], sA1[2], sU1[2], sV1[2], sname1[20], sA2[2], sU2[2], sV2[2], sname2[20], seq[2], smeters[20], sdot[2], smm[20], sM[2];

          while (!distanceFounded)
          {
            if (sscanf(msgToParse.c_str(), "%[d]%[A]%[U]%[V]%1[0-9]%[A]%[U]%[V]%1[0-9]%1[=]%[0-9]%[.]%[0-9]%[m]", sd, sA1, sU1, sV1, sname1, sA2, sU2, sV2, sname2, seq, smeters, sdot, smm, sM) == 14)
            {
              int idCharM = MOOSStrFind( msgToParse , "m");
              if (idCharM != msgToParse.size()-1)
              {
                reportEvent("iModem: le char 'm' n'est pas a la fin de la chaine["+msgToParse+"].\n");
              }
              distanceFounded = true;
              reportEvent("iModem: extracted range message: ["+msgToParse+"].\n");
              m_sLastRangeStr = msgToParse;
              m_sMsgStr = "";
              break;
            }
            else if (sscanf(msgToParse.c_str(), "%[d]%[A]%[U]%[V]%1[0-9]%[A]%[U]%[V]%1[0-9]%1[=]%[T]%[M]%[O]", sd, sA1, sU1, sV1, sname1, sA2, sU2, sV2, sname2, seq, smeters, sdot, smm) == 13)
            {
              int idCharM = MOOSStrFind( msgToParse , "O");
              if (idCharM != msgToParse.size()-1)
              {
                reportEvent("iModem: le char 'O' n'est pas a la fin de la chaine["+msgToParse+"].\n");
              }
              distanceFounded = true;
              reportEvent("iModem: extracted range message: ["+msgToParse+"].\n");
              m_sLastRangeStr = msgToParse;
              m_sMsgStr = "";
              break;
            }
            else if (msgToParse.size() > tailleMin)
            {
              msgToParse = msgToParse.substr(1, msgToParse.size());
            }
            else
            {
              break;
              // reportEvent("iModem: rien trouvé.\n");
            }
          }
        }
        // else
        // {
        //   reportEvent("iModem: string ["+msgToParse+"] trop petite.\n");
        // }
        if(distanceFounded)
        {
          sprintf(buffer,"%s=%s",m_sRobotName.c_str(),m_sLastRangeStr.c_str());
          Notify("MODEM_RANGE_MSG_RECEIVED", buffer);
        }
        else
        {
          sprintf(buffer,"%s=%s",m_sRobotName.c_str(),messageReceived.c_str());
          Notify("MODEM_MESSAGE_RECEIVED", buffer);
        }
    }
  }

  // std::cout<< "config : [" <<m_bModemConfigurationRequired<<"|"<<
  //                            m_bSentCfg <<"|"<<
  //                            m_bIsAlive<<"|"<<
  //                            m_bGetVersionData<<"|"<<
  //                            m_bGetBBUserData<<"|"<<
  //                            m_bGetFpgaVersionData<<"|"<<
  //                            m_bGetFirstPgrAck<<"|"<<
  //                            m_bGetSecondPgrAck<<"|"<<
  //                            m_bGetThirdPgrAck<<"|"<<
  //                            m_bMtReBootHasBeenSent<<"|"<<
  //                            m_bModemConfiguratonComplete<<"]"<<std::endl;

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Modem::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if (!m_MissionReader.GetValue("MODEM_SERIAL_PORT",m_portName))
    reportConfigWarning("No MODEM_SERIAL_PORT config found for " + GetAppName());
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  sParams.reverse();
  for(p = sParams.begin() ; p != sParams.end() ; p++)
  {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;
    bool handled = false;

    if(param == "BAUD_RATE_CONF")
    {
      // reportEvent("iModem: serial port baud rate conf setted to "+value+"\n");
      m_baudrate_conf = atoi(value.c_str());
      handled = true;
    }
    else if(param == "BAUD_RATE_COMM")
    {
      // reportEvent("iModem: serial port baud rate comm setted to "+value+"\n");
      m_baudrate_comm = atoi(value.c_str());
      handled = true;
    }
    else if(param == "ROBOT_NAME")
    {
      m_sRobotName = value;
      handled = true;
    }
    else if(param == "MODEM_RANGING_TIMEOUT_US")
    {
      m_uiRngTimeoutUS_param = atoi(value.c_str());
      handled = true;
    }
    else if(param == "POWERED_AT_START")
    {
      m_bIsModemPowered = MOOSStrCmp(value.c_str(),"TRUE");
      handled = true;
    }
    if(!handled)
      reportUnhandledConfigWarning(orig);
  }
  char numstr[21]={0}; // enough to hold all numbers up to 64-bits
  sprintf(numstr, "%d", m_baudrate_comm);
  // reportEvent("iModem: Openning "+m_portName+" serial port at baud rate "+numstr+"\n");
  bool portOpened = m_Port.Create(m_portName.c_str(), m_baudrate_comm);
  // reportEvent("iModem: Serial port openned\n");
  //m_Port.SetTermCharacter('\n');
  m_Port.Flush();
  m_timewarp = GetMOOSTimeWarp();

  //Power down magnet
  Notify("POWER_MODEM_EA",0.0);
  //Power down modem, they will be powered up at any reception of modem_configuration_required
  //Notify("POWER_MODEM",0.0);

  registerVariables();
  return true;
}
//---------------------------------------------------------
// Procedure: ModemTempoFunction
void Modem::ModemTempoFunction()
{
  if (!m_serial_thread_tempo.IsQuitRequested() && m_uiTimeoutUS != 0)
  {
    MOOSPause(m_uiTimeoutUS);
    m_uiTimeoutUS = 0;
  }
}
//---------------------------------------------------------
// Procedure: ListenModemMessages
void Modem::ListenModemMessages()
{
  const int buf_size = 512;
  char buf[buf_size]={0};
  string sBuf;
  char bufferNotify[100]={0};

  union HeadInf {
      char c;
      struct {
          int InCenter : 1;
          int Centered : 1;
          int Motoring : 1;
          int MotorOn  : 1;
          int Dir : 1;
          int InScan : 1;
          int NoParams : 1;
          int SentCfg : 1;
      } bits;
  } headInf;

  while (!m_serial_thread_conf.IsQuitRequested())
  {
      int msg_size = 0;
      int needed_len = SeaNetMsg::numberBytesMissing(sBuf, msg_size);
      //Waiting a message with header '@' = 0x40
      if (needed_len == SeaNetMsg::mrNotAMessage)
      {
          // Remove first character if the header cannot be decoded
          sBuf.erase(0,1);
      }
      else if (needed_len > 0)
      {
          // Read more data as needed
          int nb_read = m_Port.ReadNWithTimeOut(buf, needed_len);
          sBuf.append(buf, nb_read);
      }
      else if (needed_len == 0) //We found a message with the good header
      {
          // Process message
          MOOSTrace("iModem: Received type %d mtMessage: \n",SeaNetMsg::detectMessageType(sBuf));
          SeaNetMsg snmsg(sBuf,m_modemNodeAddr);
          snmsg.print_hex(300);

          if (snmsg.messageType() == SeaNetMsg::mtAlive)
          {
              m_bIsAlive = true;
              headInf.c = snmsg.data().at(20);

              m_bSentCfg = headInf.bits.SentCfg;
              if (!m_bSentCfg) //mtAlive HeadInf = 0x40 => config has not been sent, have to send mtSendVersion
              {
                  MOOSTrace("iModem: Modem says by mtAlive that config has not been sent (mtAlive HEadInf.sentCfg = 0) \n");
                  if (m_iInConfigTime == 7)
                  {
                    m_iInConfigTime = 8;
                    MOOSTrace("ModemManager: Modem is alive, config Process can continue\n");
                  }
              }
              else if (m_bSentCfg) //mtAlive HeadInf = 0xC0 => have to send first mtErasesector
              {
                // if(m_bGetThirdPgrAck)
                // {
                //   //The second configuration asked
                //   //Send mtReBoot
                //   SeaNetMsg_ReBoot msg_ReBoot(m_modemNodeAddr);
                //   SendModemConfigurationMessage(msg_ReBoot);
                //   MOOSTrace("iModem: Sending mtReBoot : ");
                //   msg_ReBoot.print_hex(200);
                //   SeaNetMsg_ReBoot msg_ReBootGlobal(0xff);
                //   SendModemConfigurationMessage(msg_ReBootGlobal);
                //   MOOSTrace("iModem: Sending mtReBoot Global : ");
                //   msg_ReBootGlobal.print_hex(200);
                //   m_bMtReBootHasBeenSent = true;
                //   m_iInConfigTime = 11;
                //   sprintf(bufferNotify,"%s=%i",m_sRobotName.c_str(),1);
                //   Notify("MODEM_CONFIGURATION_COMPLETE", bufferNotify);
                // }
                // else
                  MOOSTrace("iModem: Modem says by mtAlive that config has been sent (mtAlive HEadInf.sentCfg = 1) \n");
              }
          }
          else if (snmsg.messageType() == SeaNetMsg::mtVersionData)
          {
              m_bGetVersionData = true;
              MOOSTrace("iModem: Modem reply mtVersionData after receiving mtSendVersion\n");
              if (m_serial_thread_tempo.IsThreadRunning())
              {
                MOOSTrace("iModem: thread stopped when receiving mtVersionData\n");
                m_serial_thread_tempo.Stop();
                m_uiTimeoutUS = 0;
              }
          }
          else if (snmsg.messageType() == SeaNetMsg::mtBBUserData)
          {
              m_bGetBBUserData = true;
              MOOSTrace("iModem: Modem reply mtBBUserData after receiving mtSendBBUser\n");
              string modemRole = (snmsg.data().at(62))?"slave":"master";
              string modemRoleRequired = (m_iModemRoleRequired)?"slave":"master";
              MOOSTrace("iModem: Modem is currently set to %s\n",modemRole.c_str());
              MOOSTrace("iModem: Modem configuration required as %s\n",modemRoleRequired.c_str());
              MOOSTrace("iModem: Modem will now send mtFpgaVersionData\n");
          }
          else if (snmsg.messageType() == SeaNetMsg::mtFpgaVersionData)
          {
              m_bGetFpgaVersionData = true;
              MOOSTrace("iModem: Modem send mtFpgaVersionData\n");
              MOOSTrace("iModem: Modem will now send mtAlive with HeadInf = 0xC0\n");
              if (m_serial_thread_tempo.IsThreadRunning())
              {
                MOOSTrace("iModem: thread stopped when receiving mtBBUserData and mtFpgaVersionData\n");
                m_serial_thread_tempo.Stop();
                m_uiTimeoutUS = 0;
              }
          }
          else if (snmsg.messageType() == SeaNetMsg::mtPgrAck)
          {
              MOOSTrace("iModem: Modem send mtPgrAck\n");
              if(snmsg.data().at(13) == 0x01 &&snmsg.data().at(14) == 0x06 && snmsg.data().at(15) == 0x00)
              {
                  m_bGetFirstPgrAck = true;
                  MOOSTrace("iModem: Modem send FIRST mtPgrAck\n");
                  if (m_serial_thread_tempo.IsThreadRunning())
                  {
                    MOOSTrace("iModem: thread stopped when receiving FIRST mtPgrAck\n");
                    m_serial_thread_tempo.Stop();
                    m_uiTimeoutUS = 0;
                  }
              }
              else if(snmsg.data().at(13) == 0x02 &&snmsg.data().at(14) == 0x00 && snmsg.data().at(15) == 0x03)
              {
                  m_bGetSecondPgrAck = true;
                  MOOSTrace("iModem: Modem send SECOND mtPgrAck\n");
                  if (m_serial_thread_tempo.IsThreadRunning())
                  {
                    MOOSTrace("iModem: thread stopped when receiving SECOND mtPgrAck\n");
                    m_serial_thread_tempo.Stop();
                    m_uiTimeoutUS = 0;
                  }
              }
              else if(snmsg.data().at(13) == 0x02 &&snmsg.data().at(14) == 0x01 && snmsg.data().at(15) == 0x03)
              {
                  m_bGetThirdPgrAck = true;
                  MOOSTrace("iModem: Modem send THIRD mtPgrAck\n");
                  if (m_serial_thread_tempo.IsThreadRunning())
                  {
                    MOOSTrace("iModem: thread stopped when receiving THIRD mtPgrAck\n");
                    m_serial_thread_tempo.Stop();
                    m_uiTimeoutUS = 0;
                  }
              }
          }
          sBuf.erase(0,msg_size);
          //Rightafter receiving mtMessage, answer to configure Modem
          /*
          * Tritech Micron Data Modem configuration Process (see CompteRenduModemTritech.pdf)
          *  1) Modem will sent mtAlive messages every seconds with headInf bit = 0x40
          *  2) Send mtSendVersion
          *  3) Receive mtVersionData
          *  4) Send mtSendBBUser
          *  5) Receive mtBBUserData //see CompteRenduModemTritech.pdf, this reception contain Modem Configuration
          *  6) Receive mtFpgaVersionData
          *  7) Modem will sent mtAlive messages every seconds with headInf bit = = 0xc0
          *  8) Send mtEraseSector
          *  9) Receive mtPgrAck
          * 10) Send first mtProgBlock //see CompteRenduModemTritech.pdf, this progBlock contain Modem Configuration (with point 5 datas))
          * 11) Receive mtPgrAck
          * 12) Send second mtProgBlock
          * 13) Receive mtPgrAck
          * 14) Send mtReBoot
          */
          //Take a breath before talking
          MOOSPause(m_iTimeBeforeTalking);
          if (m_bIsAlive && !m_bModemConfiguratonComplete && m_iInConfigTime < 11)
          {
              if(!m_bGetVersionData && m_uiTimeoutUS == 0)
              {
                  //Send mtSendVersion
                  SeaNetMsg_SendVersion msg_SendVersion(m_modemNodeAddr);
                  SendModemConfigurationMessage(msg_SendVersion);
                  MOOSTrace("iModem: Sending mtSendVersion : ");
                  msg_SendVersion.print_hex(200);
                  m_uiTimeoutUS = 600;
                  m_serial_thread_tempo.Start();
              }
              else if(m_bGetVersionData && !m_bGetBBUserData && m_uiTimeoutUS == 0)
              {
                  //Send mtSendBBUser
                  SeaNetMsg_SendBBUser msg_SendBBUser(m_modemNodeAddr);
                  SendModemConfigurationMessage(msg_SendBBUser);
                  MOOSTrace("iModem: Sending mtSendBBUser : ");
                  msg_SendBBUser.print_hex(200);
                  m_uiTimeoutUS = 600;
                  m_serial_thread_tempo.Start();
              }
              else if(m_bGetVersionData && m_bGetBBUserData &&
                      m_bGetFpgaVersionData && m_bSentCfg &&
                      !m_bGetFirstPgrAck && m_uiTimeoutUS == 0)
              {
                  //Send mtEraseSector
                  SeaNetMsg_EraseSector msg_EraseSector(m_modemNodeAddr, 0x06, 0x00);
                  SendModemConfigurationMessage(msg_EraseSector);
                  MOOSTrace("iModem: Sending mtEraseSector : ");
                  msg_EraseSector.print_hex(200);
                  m_uiTimeoutUS = 3000;
                  m_serial_thread_tempo.Start();//A timeout of 3 seconds can be set. If the mtPgrAck is not received within this timeout period then re-send the mtEraseSector
              }
              else if(m_bGetFirstPgrAck && !m_bGetSecondPgrAck && !m_bGetThirdPgrAck && m_uiTimeoutUS == 0)
              {
                  //Send first mtProgBlock
                  char ttt = (m_iModemRoleRequired)?0x01:0x00;
                  SeaNetMsg_FirstProgBlock msg_ProgBlock(m_modemNodeAddr, 0x00, ttt);
                  SendModemConfigurationMessage(msg_ProgBlock);
                  MOOSTrace("iModem: Sending first mtProgBlock : ");
                  msg_ProgBlock.print_hex(300);
                  m_uiTimeoutUS = 2000;
                  m_serial_thread_tempo.Start();//A timeout of 2 seconds can be set. If the mtPgrAck is not received within this timeout period then re-send the mtEraseSector
              }
              else if(m_bGetFirstPgrAck && m_bGetSecondPgrAck && !m_bGetThirdPgrAck && m_uiTimeoutUS == 0)
              {
                  //Send second mtProgBlock
                  string configMsg;
                  configMsg.append(128, 0x00);
                  SeaNetMsg_SecondProgBlock msg_ProgBlock(m_modemNodeAddr, 0x01);
                  SendModemConfigurationMessage(msg_ProgBlock);
                  MOOSTrace("iModem: Sending second mtProgBlock : ");
                  msg_ProgBlock.print_hex(300);
                  m_uiTimeoutUS = 2000;
                  m_serial_thread_tempo.Start();//A timeout of 2 seconds can be set. If the mtPgrAck is not received within this timeout period then re-send the mtEraseSector
              }
              else if(m_bGetFirstPgrAck && m_bGetSecondPgrAck && m_bGetThirdPgrAck && !m_bModemConfiguratonComplete && !m_bMtReBootHasBeenSent)
              {
                  //Send mtReBoot
                  SeaNetMsg_ReBoot msg_ReBoot(m_modemNodeAddr);
                  SendModemConfigurationMessage(msg_ReBoot);
                  MOOSTrace("iModem: Sending mtReBoot : ");
                  msg_ReBoot.print_hex(200);
                  SeaNetMsg_ReBoot msg_ReBootGlobal(0xff);
                  SendModemConfigurationMessage(msg_ReBootGlobal);
                  MOOSTrace("iModem: Sending mtReBoot Global : ");
                  msg_ReBootGlobal.print_hex(200);
                  m_bMtReBootHasBeenSent = true;
                  m_iInConfigTime = 11;
                  sprintf(bufferNotify,"%s=%i",m_sRobotName.c_str(),1);
                  Notify("MODEM_CONFIGURATION_COMPLETE", bufferNotify);
              }
          }
      }
      else if(m_bModemConfiguratonComplete)
      {
          string modemRole = (m_iModemRoleRequired)?"slave":"master";
          reportEvent("iModem: Modem has been configured to "+modemRole+"\n");
      }
      else
      {
          reportEvent("iModem: No mtAlive, check connexion.\n");
      }
  }
}
//------------------------------------------------------------
// Procedure: receiveMessage
bool Modem::receiveMessage(string &message, double reception_timeout)
{
    char message_char[MESSAGE_MAX_LENGTH]={0};
    message = "";

    if(m_Port.ReadNWithTimeOut(message_char, MESSAGE_MAX_LENGTH,reception_timeout))
    {
        message = message_char;
        return true;
    }
    return false;
}

//---------------------------------------------------------
// Procedure: registerVariables

void Modem::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("MODEM_CONFIGURATION_REQUIRED", 0);
  Register("MODEM_SEND_RANGE", 0);
  Register("MODEM_ACK_RANGE", 0);
  Register("MODEM_SEND_MESSAGE", 0);
  Register("MODEM_SEND_RNG", 0);
  Register("POWERED_MODEM", 0);
  Register("POWERED_MODEM_EA", 0);
  Register("MODEM_STOP_ALIM", 0);
  // Register("MODEM_TIME_BEFORE_TALKING", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool Modem::buildReport()
{
  #if 0 // Keep these around just for template
    m_msgs << "============================================ \n";
    m_msgs << "File:                                        \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif
    m_msgs << "==============================================================\n";
    m_msgs << "iModemStatus :                                                \n";
    m_msgs << "==============================================================\n";

    ACTable actab(4);
    actab << "SerialPort | Baudrate | In config process | Role ";
    actab.addHeaderLines();
    string confProcLaunched = (m_bModemConfigurationRequired)?"yes":"no";
    string modemRoleRequired = (m_iModemRoleRequired)?"slave":"master";
    actab << m_portName << m_Port.GetBaudRate() << confProcLaunched << modemRoleRequired;
    m_msgs << actab.getFormattedString();
    m_msgs << "\n==============================================================\n";

ACTable actab2(4);
    actab2 << "ModemPower | MagnetPower | Last Message | Last Range";
    actab2.addHeaderLines();
    string modemPowered = (m_bIsModemPowered)?"yes":"no";
    string magnetPowered = (m_bIsMagnetPowered)?"yes":"no";
    actab2 << modemPowered << magnetPowered << messageReceived << rangingValue;
    m_msgs << actab2.getFormattedString();

  return true;
}
void Modem::extractMasterName(string &robotRoles)
{
      string str = MOOSChomp(robotRoles, ",");
      bool mastersNameFounded=false;
      // MOOSTrace("iModem: receiving MODEM_CONFIGURATION_REQUIRED=[%s].\n",msg.GetString().c_str());
      // MOOSTrace("iModem: str : [%s].\n",str.c_str());
      while(!mastersNameFounded)
      {
        // MOOSTrace("iModem: str inwhile : [%s].\n",str.c_str());
        if (MOOSStrFind( str , "master") != string::npos)
        {
          robotRoles = MOOSChomp(str, "=");
          MOOSTrimWhiteSpace(robotRoles);
          // MOOSTrace("iModem: extracted master : [%s].\n",m_sMasterModemName.c_str());
          mastersNameFounded = true;
          break;
        }
        else if (MOOSStrFind( str , "=") == string::npos)
        {
          mastersNameFounded = false;
          break;
        }
        str = MOOSChomp(robotRoles, ",");
      }
}
