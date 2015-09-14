/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: MapLocalizerParticleFilter.cpp                                        */
/*    DATE:                                                 */
/************************************************************/
#include <stdlib.h>
#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "particle-filter-localization/Eigen/Dense"
#include "MapLocalizerParticleFilter.h"

using namespace std;

//---------------------------------------------------------
// Constructor

MapLocalizerParticleFilter::MapLocalizerParticleFilter()
{
    this->altitudeWhereWeIgnoreGPS=-0.2;
    this->gpsEVar=25;
    this->gpsNVar=25;
    
    lastGPSENInit=false;
    lastGPSE = 0;
    lastGPSN = 0;
    lastAltitudeInit = false;
    lastAltitude = 0;
    lastVelocityInit = false;
    lastVelocity = 0;
    lastYawInit=false;
    lastYaw = 0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MapLocalizerParticleFilter::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
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

    if(key=="SONAR_DISTANCE")
    {
        // Make sure we are not in
        // the first few frames of the simulation
        if(lastYawInit&&lastAltitude)
        {
            
        }
    }
    // Receive GPS Easting/Northing
    else if(key=="GPS_EN")
    {
        lastGPSENInit=true;
        // Parse
        string s=msg.GetString();
        this->lastGPSE=atof(biteString(s,',').c_str());
        this->lastGPSN=atof(s.c_str());
        
        // If the robot is really able to receive
        // a GPS fix, update the particle filter with it
        if(lastAltitude>altitudeWhereWeIgnoreGPS)
        {
            pf.update_GPS(lastGPSE,gpsEVar,lastGPSN,gpsNVar);
        }
    }
    // Receive Keller altitude
    else if(key=="ALTITUDE")
    {
        lastAltitudeInit=true;
        lastAltitude=msg.GetDouble();
    }
    // Receive speed estimate
    else if(key=="VELOCITY_X")
    {
        lastVelocityInit=true;
        lastVelocity=msg.GetDouble();
        if(lastYawInit)
        {
            
            //pf.predict(msg.GetTime(),)
        }
    }
    // Receive IMU YAW
    else if(key=="YAW")
    {
        lastYawInit=true;
        lastYaw=msg.GetDouble();
    }
     else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool MapLocalizerParticleFilter::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool MapLocalizerParticleFilter::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool MapLocalizerParticleFilter::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "FOO") {
      handled = true;
    }
    else if(param == "BAR") {
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }

  registerVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void MapLocalizerParticleFilter::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("SONAR_DISTANCE",0);
  Register("GPS_E",0);
  Register("GPS_N",0);
  Register("ALTITUDE",0); // iKeller
  Register("VELOCITY_X",0);
  Register("YAW",0);
  // Register("FOOBAR", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool MapLocalizerParticleFilter::buildReport()
{
  m_msgs << "============================================ \n";
  m_msgs << "File:                                        \n";
  m_msgs << "============================================ \n";

  ACTable actab(4);
  actab << "Alpha | Bravo | Charlie | Delta";
  actab.addHeaderLines();
  actab << "one" << "two" << "three" << "four";
  m_msgs << actab.getFormattedString();

  return(true);
}
