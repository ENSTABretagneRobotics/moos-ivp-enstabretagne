/************************************************************/
/*    FILE: Saucisse.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "math.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "Saucisse.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Saucisse::Saucisse()
{
  m_alert_max_temperature = 999.;
  m_reset_on_startup = false;
  m_reset_all_on = true;
  m_nuc = new Nuc();

  m_status_cameras = -2;  // undefined
  m_status_modem = -2;    // undefined
  m_status_sonar = -2;    // undefined
  m_status_sounder = -2;  // undefined
  m_status_gps = -2;      // undefined

  m_left_thruster_value = 0.;
  m_right_thruster_value = 0.;
  m_vertical_thruster_value = 0.;
}

//---------------------------------------------------------
// Denstructor

Saucisse::~Saucisse()
{
  m_pololu->setAllThrustersValue(0.);
  m_pololu->bipOnExit();
  delete m_pololu;
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

    else if(key == "POWER_SOUNDER")
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
      Notify("POWER_CAMERAS", msg.GetDouble());
      Notify("POWER_GPS", msg.GetDouble());
      Notify("POWER_SOUNDER", msg.GetDouble());
      Notify("POWER_SONAR", msg.GetDouble());
      Notify("POWER_MODEM", msg.GetDouble());
      Notify("POWER_MODEM_EA", msg.GetDouble());
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
  if(current_temperature > m_alert_max_temperature)
    m_pololu->bipError();

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

    if(param == "RESET_ALL_ON")
    {
      m_reset_all_on = tolower(value) == "true";
      handled = true;
    }

    else if(param == "RESET_ON_STARTUP")
    {
      m_reset_on_startup = tolower(value) == "true";
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

  m_pololu = new Pololu(m_device_name);

  if(m_reset_on_startup)
    m_pololu->reset(m_reset_all_on,
                    m_left_thruster_value, 
                    m_right_thruster_value, 
                    m_vertical_thruster_value);

  registerVariables();  
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
  m_msgs << "============================================ \n";
  m_msgs << "iSaucisse status :                           \n";
  m_msgs << "============================================ \n";
  m_msgs << "\n";

  string error_message;
  bool pololu_ok = m_pololu->isReady(error_message);
  m_msgs << "Pololu status: \t" << (pololu_ok ? "ok" : error_message) << "\n";
  m_msgs << "\n";
  m_msgs << "Cameras: \t\t" << (m_status_cameras == -2 ? "?" : m_status_cameras + "") << "\n";
  m_msgs << "Modem: \t\t\t" << (m_status_modem == -2 ? "?" : m_status_modem + "") << "\n";
  m_msgs << "Sonar: \t\t\t" << (m_status_sonar == -2 ? "?" : m_status_sonar + "") << "\n";
  m_msgs << "Sounder: \t\t" << (m_status_sounder == -2 ? "?" : m_status_sounder + "") << "\n";
  m_msgs << "GPS: \t\t\t" << (m_status_gps == -2 ? "?" : m_status_gps + "") << "\n";
  m_msgs << "\n";
  m_msgs << "Left thruster: \t\t" << m_left_thruster_value << "\n";
  m_msgs << "Right thruster: \t" << m_right_thruster_value << "\n";
  m_msgs << "Vertical thruster: \t" << m_vertical_thruster_value << "\n";

  return  true;
}