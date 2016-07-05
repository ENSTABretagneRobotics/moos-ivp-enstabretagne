/************************************************************/
/*    FILE: SecuritySurface.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "SecuritySurface.h"

using namespace std;

//---------------------------------------------------------
// Constructor

SecuritySurface::SecuritySurface()
{
    mLastMessage = -1;
    mTimeout = 30;
    publishVariableName = "DESIRED_DEPTH";
    publishValue = "0.0";
    currentValue = "";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SecuritySurface::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if (key == publishVariableName)
        currentValue = msg.GetString();
    else
        mLastMessage = MOOSTime();

    //if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
    //  reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool SecuritySurface::OnConnectToServer()
{
  mLastMessage = MOOSTime();
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool SecuritySurface::Iterate()
{
  AppCastingMOOSApp::Iterate();

  // Do your thing here!

  int eLapsedTime = MOOSTime() - mLastMessage;

  if((eLapsedTime > mTimeout) && (currentValue != publishValue))
    Notify(publishVariableName, publishValue);

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SecuritySurface::OnStartUp()
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

    if(param == "TIMEOUT")
    {
      mTimeout = atof(value.c_str());
      handled = true;
    }

    if(param == "WATCH_VARIABLE")
    {
      variablesNames.push_back(value.c_str());
      handled = true;
    }

    if(param == "PUBLISH_VARIABLE_NAME")
    {
      publishVariableName = value.c_str();
      handled = true;
    }

    if(param == "PUBLISH_VALUE")
    {
      publishValue = value.c_str();
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

void SecuritySurface::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  for(list< string>::iterator it = variablesNames.begin(); it != variablesNames.end();it++)
  {
      Register((*it), 0);
  }
  Register(publishVariableName, 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool SecuritySurface::buildReport() 
{
  #if 0 // Keep these around just for template
    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif

    ACTable actab(3);
    actab << "Variable Name | Publish Value | Current Value";
    actab.addHeaderLines();
    actab << publishVariableName << publishValue << currentValue;
    m_msgs << actab.getFormattedString();

    ACTable actabVariables(1);
    actabVariables << "Watched Variables";
    actabVariables.addHeaderLines();

    for(list< string>::iterator it = variablesNames.begin(); it != variablesNames.end();it++)
    {
        actabVariables << (*it);
    }

    m_msgs << actabVariables.getFormattedString();

  return true;
}
