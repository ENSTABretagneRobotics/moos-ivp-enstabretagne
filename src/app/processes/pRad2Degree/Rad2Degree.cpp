/************************************************************/
/*    FILE: Rad2Degree.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Rad2Degree.h"
#include "AngleUtils.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Rad2Degree::Rad2Degree()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Rad2Degree::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "DESIRED_HEADING_DEG") 
      Notify("DESIRED_HEADING", degToRadians(msg.GetDouble()));

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Rad2Degree::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Rad2Degree::Iterate()
{
  AppCastingMOOSApp::Iterate();

  // Do your thing here!

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Rad2Degree::OnStartUp()
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

void Rad2Degree::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("DESIRED_HEADING", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool Rad2Degree::buildReport() 
{
  #if 0 // Keep these around just for template
    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif

  return true;
}
