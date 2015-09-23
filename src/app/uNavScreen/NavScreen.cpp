/************************************************************/
/*    FILE: NavScreen.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "NavScreen.h"

#define UNSET_VALUE 0

using namespace std;

//---------------------------------------------------------
// Constructor

NavScreen::NavScreen()
{
  m_moosvars["GPS_LAT"] = UNSET_VALUE;
  m_moosvars["GPS_LONG"] = UNSET_VALUE;
  m_moosvars["GPS_SIG"] = UNSET_VALUE;
  m_moosvars["SONAR_POLL"] = UNSET_VALUE;
  m_moosvars["IMU_YAW"] = UNSET_VALUE;
  m_moosvars["IMU_PITCH"] = UNSET_VALUE;
  m_moosvars["IMU_ROLL"] = UNSET_VALUE;
  m_moosvars["DEPTH"] = UNSET_VALUE;
  m_moosvars["ECHOSOUNDER_RANGE"] = UNSET_VALUE;
  m_moosvars["NAV_X"] = UNSET_VALUE;
  m_moosvars["NAV_Y"] = UNSET_VALUE;
  m_moosvars["DESIRED_HEADING"] = UNSET_VALUE;
  m_moosvars["DESIRED_DEPTH"] = UNSET_VALUE;
  m_moosvars["POWERED_CAMERAS"] = UNSET_VALUE;
  m_moosvars["POWERED_GPS"] = UNSET_VALUE;
  m_moosvars["POWERED_MINIKING"] = UNSET_VALUE;
  m_moosvars["POWERED_MICRON"] = UNSET_VALUE;
  m_moosvars["POWERED_CAMERAS"] = UNSET_VALUE;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool NavScreen::OnNewMail(MOOSMSG_LIST &NewMail)
{
  bool handle = false;
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

    for(map<string,double>::iterator it = m_moosvars.begin() ; it != m_moosvars.end() ; ++it)
      if(toupper(key) == toupper(it->first))
      {
        m_moosvars[it->first] = msg.GetDouble();
        handle = true;
        break;
      }

    if(!handle && key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool NavScreen::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool NavScreen::Iterate()
{
  AppCastingMOOSApp::Iterate();

  // Do your thing here!

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool NavScreen::OnStartUp()
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

    if(param == "FOO")
    {
      handled = true;
    }

    else if(param == "BAR")
    {
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  registerVariables();  
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void NavScreen::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  for(map<string,double>::iterator it = m_moosvars.begin() ; it != m_moosvars.end() ; ++it)
    Register(it->first, 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool NavScreen::buildReport() 
{
  m_msgs << "ATTITUDE\n";
  m_msgs << "========\n";
  ACTable actab_att(4);
  actab_att << "Dsrd Hdg | Yaw | Pitch | Roll";
  actab_att.addHeaderLines();
  actab_att << m_moosvars["DESIRED_HEADING"]
            << m_moosvars["IMU_YAW"]
            << m_moosvars["IMU_PITCH"]
            << m_moosvars["IMU_ROLL"];
  m_msgs << actab_att.getFormattedString() << "\n\n";

  m_msgs << "POSITIONING\n";
  m_msgs << "===========\n";
  ACTable actab_pos(5);
  actab_pos << "X | Y | Lat | Long | GPS signal";
  actab_pos.addHeaderLines();
  actab_pos << m_moosvars["NAV_X"]
            << m_moosvars["NAV_Y"]
            << m_moosvars["GPS_LAT"]
            << m_moosvars["GPS_LONG"] 
            << m_moosvars["GPS_SIG"];
  m_msgs << actab_pos.getFormattedString() << "\n\n";

  m_msgs << "DEPTH\n";
  m_msgs << "=====\n";
  ACTable actab_depth(3);
  actab_depth << "Depth | Dsrd depth | Bathymetry";
  actab_depth.addHeaderLines();
  actab_depth << m_moosvars["DEPTH"]
              << m_moosvars["DESIRED_DEPTH"]
              << m_moosvars["ECHOSOUNDER_RANGE"];
  m_msgs << actab_depth.getFormattedString() << "\n\n";

  m_msgs << "SONAR\n";
  m_msgs << "=====\n";
  ACTable actab_sonar(1);
  actab_sonar << "Poll";
  actab_sonar.addHeaderLines();
  actab_sonar << m_moosvars["SONAR_POLL"];
  m_msgs << actab_sonar.getFormattedString() << "\n\n";

  m_msgs << "SENSORS\n";
  m_msgs << "=======\n";
  ACTable actab_sensors(5);
  actab_sensors << "GPS" << "Modem" << "Micron" << "Miniking" << "Cameras";
  actab_sensors.addHeaderLines();
  actab_sensors << m_moosvars["POWERED_GPS"]
              << m_moosvars["POWERED_MODEM"]
              << m_moosvars["POWERED_MICRON"]
              << m_moosvars["POWERED_MINIKING"];
  m_msgs << actab_sensors.getFormattedString() << "\n\n";

  return true;
}