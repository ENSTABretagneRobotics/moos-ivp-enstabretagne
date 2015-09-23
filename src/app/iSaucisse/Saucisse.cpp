/************************************************************/
/*    FILE: Saucisse.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iostream>
#include <fstream>

#include <sstream>
#include <iterator>
#include "math.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "Saucisse.h"

#define UNDEFINED_STATUS -2

using namespace std;

//---------------------------------------------------------
// Constructor

Saucisse::Saucisse()
{
  m_autoset_on_startup = true;
  m_default_value_on_startup = true;
  m_autoset_on_quit = true;
  m_default_value_on_quit = false;

  m_nuc = new Nuc();

  m_status_cameras = UNDEFINED_STATUS;
  m_status_modem = UNDEFINED_STATUS;
  m_status_micron = UNDEFINED_STATUS;
  m_status_miniking = UNDEFINED_STATUS;
  m_status_sounder = UNDEFINED_STATUS;
  m_status_gps = UNDEFINED_STATUS;

  m_left_thruster_value = 0.;
  m_right_thruster_value = 0.;
  m_vertical_thruster_value = 0.;
}

//---------------------------------------------------------
// Destructor

Saucisse::~Saucisse()
{
  if(m_pololu->isReady())
  {
    m_pololu->setAllThrustersValue(0.);

    if(m_autoset_on_quit)
    {
      MOOSPause(100);
      m_pololu->turnOnBistableRelay(1, 0, m_default_value_on_quit);
      MOOSPause(100);
      m_pololu->turnOnBistableRelay(3, 2, m_default_value_on_quit);
      MOOSPause(100);
      m_pololu->turnOnBistableRelay(5, 4, m_default_value_on_quit);
      MOOSPause(100);
      m_pololu->turnOnBistableRelay(7, 6, m_default_value_on_quit);
      MOOSPause(100);
      m_pololu->turnOnBistableRelay(9, 8, m_default_value_on_quit);
    }

    m_pololu->bipOnExit();
  }

  delete m_pololu;
  delete m_nuc;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Saucisse::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "POWER_CAMERAS")
    {
      int success = m_pololu->turnOnBistableRelay(5, 4, (int)msg.GetDouble() == 1);
      m_status_cameras = success >= 0 ? (int)msg.GetDouble() : -1;
      Notify("POWERED_CAMERAS", m_status_cameras);
    }

    else if(key == "POWER_GPS")
    {
      int success = m_pololu->turnOnBistableRelay(3, 2, (int)msg.GetDouble() == 1);
      m_status_gps = success >= 0 ? (int)msg.GetDouble() : -1;
      Notify("POWERED_GPS", m_status_gps);
    }

    /*else if(key == "POWER_SOUNDER")
    {
      int success = m_pololu->turnOnBistableRelay(9, 8, (int)msg.GetDouble() == 1);
      m_status_sounder = success >= 0 ? (int)msg.GetDouble() : -1;
      Notify("POWERED_SOUNDER", m_status_sounder);
    }

    else if(key == "POWER_SONAR")
    {
      int success = m_pololu->turnOnBistableRelay(1, 0, (int)msg.GetDouble() == 1);
      m_status_sonar = success >= 0 ? (int)msg.GetDouble() : -1;
      Notify("POWERED_SONAR", m_status_sonar);
    }*/

    else if(key == "POWER_MICRON")
    {
      int success = m_pololu->turnOnBistableRelay(9, 8, (int)msg.GetDouble() == 1);
      m_status_micron = success >= 0 ? (int)msg.GetDouble() : -1;
      Notify("POWERED_MICRON", m_status_micron);
    }

    else if(key == "POWER_MINIKING")
    {
      int success = m_pololu->turnOnBistableRelay(1, 0, (int)msg.GetDouble() == 1);
      m_status_miniking = success >= 0 ? (int)msg.GetDouble() : -1;
      Notify("POWERED_MINIKING", m_status_miniking);
    }

    else if(key == "POWER_MODEM")
    {
      int success = m_pololu->turnOnBistableRelay(7, 6, (int)msg.GetDouble() == 1);
      m_status_modem = success >= 0 ? (int)msg.GetDouble() : -1;
      Notify("POWERED_MODEM", m_status_modem);
    }

    else if(key == "POWER_MODEM_EA")
    {
      int success = m_pololu->turnOnRelay(12, (int)msg.GetDouble() == 1);
      Notify("POWERED_MODEM_EA", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_ALL")
    {
      MOOSPause(100);
      Notify("POWER_CAMERAS", (int)msg.GetDouble());
      MOOSPause(100);
      Notify("POWER_GPS", (int)msg.GetDouble());
      MOOSPause(100);
      /*Notify("POWER_SOUNDER", (int)msg.GetDouble());
      MOOSPause(100);
      Notify("POWER_SONAR", (int)msg.GetDouble());
      MOOSPause(100);*/
      Notify("POWER_MICRON", (int)msg.GetDouble());
      MOOSPause(100);
      Notify("POWER_MINIKING", (int)msg.GetDouble());
      MOOSPause(100);
      Notify("POWER_MODEM", (int)msg.GetDouble());
      MOOSPause(100);
      Notify("POWER_MODEM_EA", (int)msg.GetDouble());
      MOOSPause(100);
    }

    else if(key == "EMIT_BIPS")
    {
      int success = m_pololu->emitBips((int)msg.GetDouble());
    }

    else if(key == "SET_THRUSTERS_FORCE_MIN")
    {
      Notify("SET_THRUSTER_LEFT", -1.);
      Notify("SET_THRUSTER_RIGHT", -1.);
      Notify("SET_THRUSTER_VERTICAL", -1.);
    }

    else if(key == "SET_THRUSTERS_FORCE_NEUTRAL")
    {
      Notify("SET_THRUSTER_LEFT", 0.);
      Notify("SET_THRUSTER_RIGHT", 0.);
      Notify("SET_THRUSTER_VERTICAL", 0.);
    }

    else if(key == "SET_THRUSTERS_FORCE_MAX")
    {
      Notify("SET_THRUSTER_LEFT", 1.);
      Notify("SET_THRUSTER_RIGHT", 1.);
      Notify("SET_THRUSTER_VERTICAL", 1.);
    }

    else if(key == "SET_THRUSTER_LEFT")
    {
      int success = m_pololu->setLeftThrusterValue(msg.GetDouble());
      m_left_thruster_value = msg.GetDouble();
    }

    else if(key == "SET_THRUSTER_RIGHT")
    {
      int success = m_pololu->setRightThrusterValue(msg.GetDouble());
      m_right_thruster_value = msg.GetDouble();
    }

    else if(key == "SET_THRUSTER_VERTICAL")
    {
      int success = m_pololu->setVerticalThrusterValue(msg.GetDouble());
      m_vertical_thruster_value = msg.GetDouble();
    }

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Saucisse::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Saucisse::Iterate()
{
  AppCastingMOOSApp::Iterate();

  string error_message;
  bool pololu_ok = m_pololu->isReady(error_message);
  Notify("SAUCISSE_POLOLU_STATUS", pololu_ok ? "ok" : error_message);

  double current_temperature = m_nuc->getTemperature();
  Notify("NUC_TEMPERATURE", current_temperature);

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Saucisse::OnStartUp()
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

    if(param == "AUTOSET_ON_STARTUP")
    {
      string str_autoset_on_startup = "true", str_default_value_on_startup = "true";
      value = tolower(value);
      MOOSValFromString(str_autoset_on_startup, value, "autoset");
      MOOSValFromString(str_default_value_on_startup, value, "all_on");
      m_autoset_on_startup = str_autoset_on_startup == "true";
      m_default_value_on_startup = str_default_value_on_startup == "true";
      handled = true;
    }

    else if(param == "AUTOSET_ON_QUIT")
    {
      string str_autoset_on_quit = "true", str_default_value_on_quit = "false";
      value = tolower(value);
      MOOSValFromString(str_autoset_on_quit, value, "autoset");
      MOOSValFromString(str_default_value_on_quit, value, "all_on");
      m_autoset_on_quit = str_autoset_on_quit == "true";
      m_default_value_on_quit = str_default_value_on_quit == "true";
      handled = true;
    }

    else if(param == "DEVICE_NAME")
    {
      m_device_name = value;
      handled = true;
    }
    
    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  registerVariables();
  m_pololu = new Pololu(m_device_name);

  if(!m_pololu->isReady())
    reportRunWarning("Error on Pololu connection.");

  Notify("SET_THRUSTER_LEFT", m_left_thruster_value);
  Notify("SET_THRUSTER_RIGHT", m_right_thruster_value);
  Notify("SET_THRUSTER_VERTICAL", m_vertical_thruster_value);

  if(m_autoset_on_startup)
    Notify("POWER_ALL", m_default_value_on_startup ? 1 : 0);

  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void Saucisse::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("POWER_*", "*", 0);
  Register("EMIT_BIPS", 0);
  Register("SET_THRUSTER*", "*", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool Saucisse::buildReport() 
{
  string error_message;
  bool pololu_ok = m_pololu->isReady(error_message);
  m_msgs << "Pololu status: \t" << (pololu_ok ? "ok" : error_message) << "\n";
  m_msgs << "\n";

  ACTable actab_relays(2);
  actab_relays << "Relay" << "On";
  actab_relays.addHeaderLines();
  actab_relays << "Cameras" << sensorStatusInText(m_status_cameras);
  actab_relays << "Modem" << sensorStatusInText(m_status_modem);
  actab_relays << "Micron" << sensorStatusInText(m_status_micron);
  actab_relays << "Miniking" << sensorStatusInText(m_status_miniking);
  actab_relays << "GPS" << sensorStatusInText(m_status_gps);
  m_msgs << actab_relays.getFormattedString() << "\n\n";

  ACTable actab_thrusters(2);
  actab_thrusters << "Thruster" << "Value";
  actab_thrusters.addHeaderLines();
  actab_thrusters << "Left" << m_left_thruster_value;
  actab_thrusters << "Right" << m_right_thruster_value;
  actab_thrusters << "Vertical" << m_vertical_thruster_value;
  m_msgs << actab_thrusters.getFormattedString() << "\n\n";

  ACTable actab_temperatures(2);
  actab_temperatures << "Unit" << "°C";
  actab_temperatures.addHeaderLines();
  actab_temperatures << "NUC" << m_nuc->getTemperature();
  m_msgs << actab_temperatures.getFormattedString() << "\n\n";
  
  return true;
}

//------------------------------------------------------------
// Procedure: sensorStatusInText()

std::string Saucisse::sensorStatusInText(int status)
{
  if(status == UNDEFINED_STATUS)
    return "?";

  else
  {
    stringstream convert;
    convert << status;
    return convert.str();
  }
}