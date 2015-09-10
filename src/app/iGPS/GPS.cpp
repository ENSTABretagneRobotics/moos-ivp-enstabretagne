/************************************************************/
/*    FILE: GPS.cpp
/*    ORGN: ENSTA Bretagne
/*    AUTH: Clement Aubry (from MSIS iGPS app)
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "GPS.h"

using namespace std;

//---------------------------------------------------------
// Constructor

GPS::GPS()
{
  m_iterations = 0;
  m_timewarp   = 1;
  m_portName = "/dev/ttyUSB0";
  m_baudrate = 4800;
  m_bPublishRaw = true;

  m_dSpeed = 0;
  m_dHeading = 0;
  m_dYaw = 0;
  m_dN = 0;
  m_dE = 0;
  m_dX = 0;
  m_dY = 0;
  m_dLongitude = 0;
  m_dLatitude = 0;
  m_dNbSat = 0;
  m_dAltitude = 0;
  m_bIsPowered = false;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool GPS::OnNewMail(MOOSMSG_LIST &NewMail)
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
    else if(key == "POWERED_GPS")
      m_bIsPowered= msg.GetDouble();

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool GPS::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GPS::Iterate()
{
  AppCastingMOOSApp::Iterate();

  m_iterations++;
  if (m_bIsPowered)
    GetData();

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GPS::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  double dLatOrigin;
  if (!m_MissionReader.GetValue("LatOrigin",dLatOrigin))
  {
    reportConfigWarning("LatOrigin not set!!!");
    return false;
  }

  double dLongOrigin;
  if (!m_MissionReader.GetValue("LongOrigin",dLongOrigin))
  {
    reportConfigWarning("LongOrigin not set!!!");
    return false;
  }

  if (!m_Geodesy.Initialise(dLatOrigin,dLongOrigin))
  {
    reportConfigWarning("Geodesy initialisation failed!!!");
    return false;
  }
  if (!m_MissionReader.GetValue("GPS_SERIAL_PORT",m_portName))
    reportConfigWarning("No GPS_SERIAL_PORT config found for " + GetAppName());

  int max_retries = 5;
  double dGPSPeriod = 1.0;

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

    if(param == "MAX_RETRIES")
    {
      max_retries = atoi(value.c_str());
      handled = true;
    }
    else if(param == "BAUDRATE")
    {
      // reportEvent("iModem: serial port baud rate conf setted to "+value+"\n");
      m_baudrate = atoi(value.c_str());
      handled = true;
    }
    else if(param == "PUBLISH_RAW")
    {
      // reportEvent("iModem: serial port baud rate conf setted to "+value+"\n");
      m_bPublishRaw = MOOSStrCmp(value.c_str(),"TRUE");
      handled = true;
    }
    else if(param == "POWERED_AT_START")
    {
      m_bIsPowered = MOOSStrCmp(value.c_str(),"TRUE");
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  m_timewarp = GetMOOSTimeWarp();

  bool portOpened = m_Port.Create(m_portName.c_str(), m_baudrate);
  m_Port.Flush();

  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void GPS::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("POWERED_GPS", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool GPS::buildReport()
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
    m_msgs << "iGPS Status :                                                \n";
    m_msgs << "==============================================================\n";

    ACTable actab(6);
    actab << "Time | Speed | Heading | Yaw | N | E ";
    actab.addHeaderLines();
    actab << m_dGpsTime << m_dSpeed << m_dHeading << m_dYaw << m_dN << m_dE;
    m_msgs << actab.getFormattedString();
    m_msgs << "\n==============================================================\n";

ACTable actab2(6);
    actab2 << "Nb Sat | Latitude | Longitude | Altitude | X | Y ";
    actab2.addHeaderLines();
    actab2 << m_dNbSat << m_dLatitude << m_dLongitude << m_dAltitude << m_dX << m_dY;
    m_msgs << actab2.getFormattedString();

  return true;
}
bool GPS::InitialiseSensor()
{
  MOOSPause(1000);
  if (m_Port.Flush()==-1)
  {
    return false;
  } else return true;
}

bool GPS::GetData()
{
  string sMessage;
  double dTime;

  if(m_Port.IsStreaming())
  {
    if (!m_Port.GetLatest(sMessage,dTime))
    {
      return false;
    }
  } else {
    if (!m_Port.GetTelegram(sMessage,0.5))
    {
      return false;
    }
  }
  if (m_bPublishRaw)
  {
    // SetMOOSVar("Raw",sMessage,MOOSTime());
    Notify("GPS_RAW",sMessage);
  }

  return ParseNMEAString(sMessage);
}

bool GPS::ParseNMEAString(string &sNMEAString)
{
  if(!CMOOSInstrument::DoNMEACheckSum(sNMEAString))
  {
    // MOOSDebugWrite("GPS NMEA checksum failed!");
      reportRunWarning("GPS NMEA checksum failed!");
    return false;
  }
  else
    retractRunWarning("GPS NMEA checksum failed!");
  string sCopy = sNMEAString;
  string sHeader = MOOSChomp(sNMEAString,",");
  bool bQuality = true;

  if (sHeader == "$GPRMC")
  {
    double dTimeMOOS = MOOSTime();
    // will only be used for speed and heading!
    string sTmp = MOOSChomp(sNMEAString,","); //UTC
    double dTime = atof(sTmp.c_str());
    m_dGpsTime = dTime;
    Notify("GPS_TIME",dTime);
    sTmp = MOOSChomp(sNMEAString, ","); // Validity
    if (sTmp == "V")
      bQuality = false;
    sTmp = MOOSChomp(sNMEAString, ","); // Lat
    sTmp = MOOSChomp(sNMEAString, ","); // N/S
    sTmp = MOOSChomp(sNMEAString, ","); // Long
    sTmp = MOOSChomp(sNMEAString, ","); // E/W

    sTmp = MOOSChomp(sNMEAString, ","); // Speed in knots
    double dSpeed = atof(sTmp.c_str());

    sTmp = MOOSChomp(sNMEAString, ","); // True course
    double dHeading = atof(sTmp.c_str());

    if (bQuality)
    {
      dSpeed = Knots2MPS(dSpeed);
      // SetMOOSVar("Speed",dSpeed,dTimeMOOS);
      Notify("GPS_SPEED",dSpeed);
      m_dSpeed = dSpeed;

      while(dHeading > 180) dHeading -= 360;
      while(dHeading < -180) dHeading += 360;
      double dYaw = -dHeading*M_PI/180.0;
      // SetMOOSVar("Heading",dHeading,dTimeMOOS);
      Notify("GPS_HEADING",dHeading);
      m_dHeading= dHeading;
      // SetMOOSVar("Yaw",dYaw,dTimeMOOS);
      Notify("GPS_YAW",dYaw);
      m_dYaw = dYaw;
    }


  } else if (sHeader == "$GPGGA") {
    double dLat, dLong;
    double dTimeMOOS = MOOSTime();
    string sTmp = MOOSChomp(sNMEAString,",");

    // First time
    double dTime = atof(sTmp.c_str());
    m_dGpsTime = dTime;
    Notify("GPS_TIME",dTime);
    // then Latitude
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
    {
      bQuality = false;
      // MOOSTrace("NMEA message received with no Latitude.");
      reportRunWarning("NMEA message received with no Latitude.");
      return false;
    } else {
      retractRunWarning("NMEA message received with no Latitude.");
      dLat = atof(sTmp.c_str());
      sTmp = MOOSChomp(sNMEAString, ",");
      string sNS = sTmp;
      if (sNS == "S")
        dLat *= -1.0;
    }

    // then Longitude
    sTmp = MOOSChomp(sNMEAString, ",");
    if (sTmp.size() == 0)
    {
      bQuality = false;
      // MOOSTrace("NMEA message received with no Longitude.");
      reportRunWarning("NMEA message received with no Longitude.");
      return false;
    } else {
      retractRunWarning("NMEA message received with no Longitude.");
      dLong = atof(sTmp.c_str());
      sTmp = MOOSChomp(sNMEAString, ",");
      string sEW = sTmp;
      if (sEW == "W")
        dLong *= -1.0;
    }

    // then GPS FIX Verification
    sTmp = MOOSChomp(sNMEAString, ",");
    int iFix = atoi(sTmp.c_str());
    if (iFix == 0)
      bQuality = false;

    // then number of stellites
    sTmp = MOOSChomp(sNMEAString, ",");
    int iSatellites = atoi(sTmp.c_str());
    if (iSatellites < 4)
      bQuality = false;

    // then Horizontal Dilution of Precision HDOP
    sTmp = MOOSChomp(sNMEAString, ",");
    double dHDOP = atof(sTmp.c_str());

    // then altitude above mean sea level
    sTmp = MOOSChomp(sNMEAString, ",");
    double dAltitude = atof(sTmp.c_str());
    sTmp = MOOSChomp(sNMEAString, ","); //removes M of meters.

    // then height of geoid above WGS84 ellipsoid
    sTmp = MOOSChomp(sNMEAString, ",");
    double dHeightGeoid = atof(sTmp.c_str());
    sTmp = MOOSChomp(sNMEAString, ","); //removes M of meters.

    // then time since last DGPS and DGPS ID
    // ignored

    // Conversion
    dLat = DMS2DecDeg(dLat);
    dLong = DMS2DecDeg(dLong);

    double dXLocal; // X coordinate in local
    double dYLocal; // Y coordinate in local
    double dNLocal; // Northing in local
    double dELocal; // Easting in local

    if (bQuality)
    {
      if (m_Geodesy.LatLong2LocalUTM(dLat,dLong,dNLocal,dELocal))
      {
        if (bQuality)
        {
          // SetMOOSVar("N",dNLocal,dTimeMOOS);
          Notify("GPS_N",dNLocal);
          m_dN = dNLocal;
          // SetMOOSVar("E",dELocal,dTimeMOOS);
          Notify("GPS_E",dELocal);
          m_dE = dELocal;
        }
      }
      if (m_Geodesy.LatLong2LocalUTM(dLat,dLong,dYLocal,dXLocal))
      {
        if (bQuality)
        {
          // SetMOOSVar("X",dXLocal,dTimeMOOS);
          Notify("GPS_X",dXLocal);
          m_dX = dXLocal;
          // SetMOOSVar("Y",dYLocal,dTimeMOOS);
          Notify("GPS_Y",dYLocal);
          m_dY = dYLocal;
        }
      }
      // SetMOOSVar("Longitude",dLong,dTimeMOOS);
      Notify("GPS_LONGITUDE",dLong);
      m_dLongitude = dLong;
      // SetMOOSVar("Latitude",dLat,dTimeMOOS);
      Notify("GPS_LATITUDE",dLat);
      m_dLatitude = dLat;
      // SetMOOSVar("Satellites",iSatellites,dTimeMOOS);
      Notify("GPS_NB_SAT",iSatellites);
      m_dNbSat = iSatellites;
      // SetMOOSVar("Altitude",dAltitude,dTimeMOOS);
      Notify("GPS_ALTITUDE",dAltitude);
      m_dAltitude = dAltitude;
    }
  }
  return true;
}


double GPS::DMS2DecDeg(double dfVal)
{
  int nDeg = (int)(dfVal/100.0);

  double dfTmpDeg = (100.0*(dfVal/100.0-nDeg))/60.0;

  return  dfTmpDeg+nDeg;
}

double GPS::Knots2MPS(double speed)
{
  return speed*KNOTS2MPS;
}
