/************************************************************/
/*    FILE: Keller.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Keller.h"
#include "kellermsg.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Keller::Keller()
{
  m_port_is_initialized = false;
  m_lastP_value = -1.0;
  m_lastT_value = -1.0;
  m_iMmaxRetries = 1;
  m_bTemperatureRequested = false;

  m_bKellerInitialized = false;

  kellerPressureRequest = KellerMsg_ReadOutPressureFloatRequest();
  kellerTemperatureRequest = KellerMsg_ReadOutTemperatureFloatRequest();
}

//---------------------------------------------------------
// Destructor

Keller::~Keller()
{
  if(m_port_is_initialized)
    m_serial_port.Close();
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Keller::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "GET_KELLER_TEMPERATURE")
    {
      m_bTemperatureRequested = msg.GetDouble();
    }
    else if(key == "FOO")
      cout << "great!";

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Keller::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second
union uFloat
{
  float v;
  uint8 c[4];
};
bool Keller::Iterate()
{
  AppCastingMOOSApp::Iterate();

  if (m_port_is_initialized)
  {
    if(m_bKellerInitialized)
    {
      ReadPressure();
      if (m_bTemperatureRequested)
        ReadTemperature();
    }
  }

  AppCastingMOOSApp::PostReport();
  return true;
}
bool Keller::ReadPressure()
{
  const int buf_size = 15;
  char buf[buf_size];
  uint8 bufui8[buf_size];
  uint8 crc_h = 0;
  uint8 crc_l = 0;
  bool flag = true;
  uFloat value;

  SendKellerMessage(kellerPressureRequest);
  // msgInitKeller.print_hex();
  MOOSPause(1);

  int nb_read = m_serial_port.ReadNWithTimeOut(buf, 5);
  // printf("\nreception : ");
  // for (uint k = 0;k<nb_read;k++)
  // {
  //   printf("\n\t%02x",(uint8)buf[k]);
  // }
  //Vérifications
  if ((uint8)buf[0] != (uint8)0xfa)
    flag=false;
  else if((uint8)buf[1] != (uint8)0x49)
    flag=false;
  else if((uint8)buf[2] != (uint8)0x01)
    flag=false;
  else if ((uint8)buf[3] != (uint8)0xa1)
    flag=false;
  else if ((uint8)buf[4] != (uint8)0xa7)
    flag=false;
  // printf("\n\techo recu");

  if (flag){
    MOOSPause(2);
    nb_read = m_serial_port.ReadNWithTimeOut(buf, 9);

    // printf("\nreception : ");
    for (uint k = 0;k<nb_read;k++)
    {
      // printf("\n\t%02x",(uint8)buf[k]);
      bufui8[k] = (uint8)buf[k];
    }
    //Vérifications
    if ((uint8)bufui8[0] != (uint8)0xfa)
      flag=false;
    else if((uint8)buf[1] != (uint8)0x49)
      flag=false;
    if (flag){
      CalcCRC16(bufui8, 9-2, &crc_h, &crc_l);
      // printf("\n\tcrc_h%02x",crc_h);
      // printf("\n\tcrc_l%02x",crc_l);
      // CRC-16.
      if (((uint8)buf[7] != crc_h)||((uint8)buf[8] != crc_l))
      {
        printf("Error reading Pressure data from a P33x : Bad CRC-16. \n");
      }
      else
      {
        value.c[3] = (uint8)buf[2];
        value.c[2] = (uint8)buf[3];
        value.c[1] = (uint8)buf[4];
        value.c[0] = (uint8)buf[5];
        // printf("value read : %f\n",value.v);
        m_lastP_value = value.v;
        Notify("KELLER_PRESSURE",m_lastP_value);
      }
    }
  }
  else
  {
    printf("Error reading Pressure value, receiving echo\n");
  }
}
bool Keller::ReadTemperature()
{
  const int buf_size = 15;
  char buf[buf_size];
  uint8 bufui8[buf_size];
  uint8 crc_h = 0;
  uint8 crc_l = 0;
  bool flag = true;
  uFloat value;

  SendKellerMessage(kellerTemperatureRequest);
  // msgInitKeller.print_hex();
  MOOSPause(1);

  int nb_read = m_serial_port.ReadNWithTimeOut(buf, 5);
  // printf("\nreception : ");
  // for (uint k = 0;k<nb_read;k++)
  // {
  //   printf("\n\t%02x",(uint8)buf[k]);
  // }
  //Vérifications
  if ((uint8)buf[0] != (uint8)0xfa)
    flag=false;
  else if((uint8)buf[1] != (uint8)0x49)
    flag=false;
  else if((uint8)buf[2] != (uint8)0x04)
    flag=false;
  for (uint k = 0;k<nb_read;k++)
  {
    // printf("\n\t%02x",(uint8)buf[k]);
    bufui8[k] = (uint8)buf[k];
  }
  //Vérifications
  if (flag){
    CalcCRC16(bufui8, 5-2, &crc_h, &crc_l);
    if (((uint8)buf[3] != crc_h)||((uint8)buf[4] != crc_l))
    {
      printf("Error reading temperature data from a P33x : Bad CRC-16. \n");
      flag=false;
    }
    // printf("\n\techo recu");

    if (flag){
      MOOSPause(2);
      nb_read = m_serial_port.ReadNWithTimeOut(buf, 9);

      // printf("\nreception : ");
      for (uint k = 0;k<nb_read;k++)
      {
        // printf("\n\t%02x",(uint8)buf[k]);
        bufui8[k] = (uint8)buf[k];
      }
      //Vérifications
      if ((uint8)bufui8[0] != (uint8)0xfa)
        flag=false;
      else if((uint8)buf[1] != (uint8)0x49)
        flag=false;
      if (flag){
        CalcCRC16(bufui8, 9-2, &crc_h, &crc_l);
        // printf("\n\tcrc_h%02x",crc_h);
        // printf("\n\tcrc_l%02x",crc_l);
        // CRC-16.
        if (((uint8)buf[7] != crc_h)||((uint8)buf[8] != crc_l))
        {
          printf("Error reading temperature data from a P33x : Bad CRC-16. \n");
        }
        else
        {
          value.c[3] = (uint8)buf[2];
          value.c[2] = (uint8)buf[3];
          value.c[1] = (uint8)buf[4];
          value.c[0] = (uint8)buf[5];
          // printf("value read : %f\n",value.v);
          m_lastT_value = value.v;
          Notify("KELLER_TEMPERATURE",m_lastT_value);
        }
      }
    }
    else
    {
      printf("Error reading value, receiving echo\n");
    }
  }
}
//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Keller::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
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

    if(param == "SERIAL_PORT_NAME")
    {
      m_port_name = value;
      handled = true;
    }
    else if(param == "MAX_RETRIES")
    {
      m_iMmaxRetries = atoi(value.c_str());
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  m_port_is_initialized = initSerialPort();
  if(!m_port_is_initialized){
    reportConfigWarning("Initialization failed on " + m_port_name);
  }
  else
  {
    m_bKellerInitialized = initKeller(m_iMmaxRetries);
    if(!m_bKellerInitialized)
      reportConfigWarning("Init keller failed");
  }
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void Keller::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("GET_KELLER_TEMPERATURE", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool Keller::buildReport()
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
    m_msgs << "============================================ \n";
    m_msgs << "iKeller Status:                              \n";
    m_msgs << "============================================ \n";

    ACTable actab(5);
    actab << "Serial Port | Serial Initialized | Keller Initialized | Last Pressure value | Last Temperature value";
    actab.addHeaderLines();
    actab << m_port_name << m_port_is_initialized << m_bKellerInitialized << m_lastP_value << m_lastT_value;
    m_msgs << actab.getFormattedString();

  return true;
}

//------------------------------------------------------------
// Procedure: initSerialPort

bool Keller::initSerialPort()
{
  bool portOpened = m_serial_port.Create(m_port_name.c_str(), 9600);
  // reportEvent("iModem: Serial port openned\n");
  //m_Port.SetTermCharacter('\n');
  m_serial_port.Flush();
  return portOpened;
}

bool Keller::initKeller(int maxRetries)
{
  const int buf_size = 15;
  char buf[buf_size];
  uint8 bufui8[buf_size];
  uint8 crc_h = 0;
  uint8 crc_l = 0;

  KellerMsg_InitDeviceRequest msgInitKeller;
  for (int k=0;k<maxRetries;k++)
  {
    int flag = true;
    // printf("\nenvoi init : ");
    SendKellerMessage(msgInitKeller);
    // msgInitKeller.print_hex();
    MOOSPause(2);

    int nb_read = m_serial_port.ReadNWithTimeOut(buf, 4);
    // printf("\nreception : ");
    // for (uint k = 0;k<nb_read;k++)
    // {
    //   printf("\n\t%02x",(uint8)buf[k]);
    // }
    //Vérifications
    if ((uint8)buf[0] != (uint8)0xfa)
      flag=false;
    else if((uint8)buf[1] != (uint8)0x30)
      flag=false;
    else if((uint8)buf[2] != (uint8)0x04)
      flag=false;
    else if ((uint8)buf[3] != (uint8)0x43)
      flag=false;
    // printf("\n\techo recu");

    if (flag){
      MOOSPause(2);
      nb_read = m_serial_port.ReadNWithTimeOut(buf, 10);

      // printf("\nreception : ");
      for (uint k = 0;k<nb_read;k++)
      {
        // printf("\n\t%02x",(uint8)buf[k]);
        bufui8[k] = (uint8)buf[k];
      }
      //Vérifications
      if ((uint8)bufui8[0] != (uint8)0xfa)
        flag=false;
      else if((uint8)buf[1] != (uint8)0x30)
        flag=false;
      if (flag){
        CalcCRC16(bufui8, 10-2, &crc_h, &crc_l);
        // printf("\n\tcrc_h%02x",crc_h);
        // printf("\n\tcrc_l%02x",crc_l);
        // CRC-16.
        if (((uint8)buf[8] != crc_h)||((uint8)buf[9] != crc_l))
        {
          printf("Error reading data from a P33x : Bad CRC-16. \n");
        }
        else
          return true;
      }
    }
  }
  return false;
}
