/************************************************************/
/*    FILE: NavigationScreen.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "NavigationScreen.h"

#define UNSET_VALUE "-"

using namespace std;

//---------------------------------------------------------
// Constructor

NavigationScreen::NavigationScreen()
{
  m_moosvars["GPS_LAT"] = UNSET_VALUE;
  m_moosvars["GPS_LONG"] = UNSET_VALUE;
  m_moosvars["GPS_SIG"] = UNSET_VALUE;
  m_moosvars["SONAR_POLL"] = UNSET_VALUE;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool NavigationScreen::OnNewMail(MOOSMSG_LIST &NewMail)
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

    for(map<string,string>::iterator it = m_moosvars.begin() ; it != m_moosvars.end() ; ++it)
      if(toupper(key) == toupper(it->first))
        m_moosvars[key] = msg.GetString();

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool NavigationScreen::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool NavigationScreen::Iterate()
{
  AppCastingMOOSApp::Iterate();

  // Do your thing here!

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool NavigationScreen::OnStartUp()
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

void NavigationScreen::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  for(map<string,string>::iterator it = m_moosvars.begin() ; it != m_moosvars.end() ; ++it)
    Register(it->first, 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool NavigationScreen::buildReport() 
{
  m_msgs << "GPS                                          \n";
  m_msgs << "-------------------------------------------- \n";
  ACTable actab_gps(3);
  actab_gps << "Lat | Long | Signal";
  actab_gps.addHeaderLines();
  actab_gps << m_moosvars["GPS_LAT"] << m_moosvars["GPS_LONG"] << m_moosvars["GPS_SIG"];
  m_msgs << actab_gps.getFormattedString();

  ACTable actab_sonar(1);
  actab_sonar << "Poll";
  actab_sonar.addHeaderLines();
  actab_sonar << m_moosvars["SONAR_POLL"];
  m_msgs << actab_sonar.getFormattedString();

  return true;
}