/************************************************************/
/*    FILE: EchoSounder.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "EchoSounder.h"

using namespace std;

//---------------------------------------------------------
// Constructor

EchoSounder::EchoSounder()
{
  m_port_is_initialized = false;
  m_last_value = -2.;
}

//---------------------------------------------------------
// Destructor

EchoSounder::~EchoSounder()
{
  if(m_port_is_initialized)
    m_serial_port.Close();
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool EchoSounder::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "FOO")
      cout << "great!";

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool EchoSounder::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool EchoSounder::Iterate()
{
  AppCastingMOOSApp::Iterate();

  if(m_port_is_initialized)
    Notify("ECHOSOUNDER_RANGE", getRange());

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool EchoSounder::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if (!m_MissionReader.GetValue("ECHOSOUNDER_SERIAL_PORT",m_port_name))
    reportConfigWarning("No ECHOSOUNDER_SERIAL_PORT config found for " + GetAppName());
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

    // if(param == "SERIAL_PORT_NAME")
    // {
    //   handled = true;
    //   if(!initSerialPort(value))
    //     reportConfigWarning("Initialization failed on " + value);
    // }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  if(!initSerialPort())
    reportConfigWarning("Initialization failed on [" + m_port_name+"]");

  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void EchoSounder::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool EchoSounder::buildReport()
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
    m_msgs << "iEchoSounder status:                         \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Serial Port | Baudrate | Port initialized | Depth";
    actab.addHeaderLines();
    string portInit = (m_port_is_initialized)?"yes":"no";
    actab << m_port_name << m_serial_port.GetBaudRate() << portInit << m_last_value;
    m_msgs << actab.getFormattedString();

  return true;
}

//------------------------------------------------------------
// Procedure: initSerialPort

bool EchoSounder::initSerialPort()
{
  ifstream test_file(m_port_name.c_str());
  if(test_file.fail())
    return false;

  #ifdef _WIN32
    m_serial_port = MOOSNTSerialPort();
  #else
    m_serial_port = CMOOSLinuxSerialPort();
  #endif

  m_port_is_initialized = m_serial_port.Create((char*)m_port_name.c_str());
  return m_port_is_initialized;
}

//------------------------------------------------------------
// Procedure: getRange

double EchoSounder::getRange()
{
  int result;
  double range;

  if(!m_port_is_initialized)
    return -1;

  char echo_value[ECHOSOUNDER_MAX_STRING_LENGTH];
  result = m_serial_port.ReadNWithTimeOut(echo_value, ECHOSOUNDER_MAX_STRING_LENGTH);

  // Example of returned value: "001.567m"
  if(!dataIsValid(echo_value))
  {
    reportRunWarning("Data not valid");
    return -2;
  }
  else
    retractRunWarning("Data not valid");

  if(!result)
  {
    reportRunWarning("No result received");
    return -1;
  }
  else
    retractRunWarning("No result received");

  char parsed_value[8];

  // The beginning of the string is kept
  for(int i = 0 ; i < 7 ; i ++)
    parsed_value[i] = echo_value[i];
  parsed_value[7] = '\0';

  // Conversion into decimals
  range = atof(parsed_value);
  m_last_value = range;
  return range;
}

//------------------------------------------------------------
// Procedure: dataIsValid

bool EchoSounder::dataIsValid(string data)
{
  return data.find('.') && data.find('m');
}
