/**
 * \file GPSoE.cpp
 * \brief Classe GPSoE
 * \author Team CISSAU - Veni Vidi Vici (ENSTA Bretagne)
 * \version 0.1
 * \date Jun 7th 2013
 *
 * Application MOOS donnant les consignes de déplacement de l'AUV
 *
 */

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "GPSoE.h"

using namespace std;

GPSoE::GPSoE()
{
	m_iterations = 0;
	m_timewarp   = 1;

	hGPSoEMgr = INVALID_HGPSOEMGR_VALUE;
	//InitGPSoEMgr(&hGPSoEMgr, (char*)SZDEV, 100, DATA_POLLING_MODE_GPSOE, THREAD_PRIORITY_NORMAL, TRUE);

  m_dGpsTime = 0;
  m_dLongitude = 0;
  m_dLatitude = 0;

  m_sGpsAddr = "192.168.0.34:4001";
  m_sLatOrigin = 0;
  m_sLongOrigin = 0;
}

GPSoE::~GPSoE()
{
  	//ReleaseGPSoEMgr(&hGPSoEMgr);
}

bool GPSoE::OnNewMail(MOOSMSG_LIST &NewMail)
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

  return(true);
}

bool GPSoE::OnConnectToServer()
{
  registerVariables();
  return true;
}

bool GPSoE::Iterate()
{
  AppCastingMOOSApp::Iterate();
	m_iterations++;

	HGPSOE hGPSoE = INVALID_HGPSOE_VALUE;

	OpenGPSoE(&hGPSoE, (char *)m_sGpsAddr.c_str());

	GetDataGPSoE(hGPSoE, &m_dGpsTime, &m_dLatitude, &m_dLongitude);
	double moos_time = MOOSTime();

	CloseGPSoE(&hGPSoE);

  // Conversion
  // m_dLatitude = DMS2DecDeg(m_dLatitude);
  // m_dLongitude = DMS2DecDeg(m_dLongitude);

	//From iGPS, to be coherent: same outputs
  double dXLocal; // X coordinate in local
  double dYLocal; // Y coordinate in local
  double dNLocal; // Northing in local
  double dELocal; // Easting in local

	Notify("GPSOE_LAT", m_dLatitude, moos_time);
	Notify("GPSOE_LON", m_dLongitude, moos_time);
	Notify("GPSOE_UTC", m_dGpsTime, moos_time);
  if (m_Geodesy.LatLong2LocalUTM(m_dLatitude,m_dLongitude,dNLocal,dELocal))
  {
      Notify("GPSOE_N",dNLocal, moos_time);
      m_dN = dNLocal;
      Notify("GPSOE_E",dELocal, moos_time);
      m_dE = dELocal;
  }
  if (m_Geodesy.LatLong2LocalUTM(m_dLatitude,m_dLongitude,dYLocal,dXLocal))
  {
      Notify("GPSOE_X",dXLocal, moos_time);
      m_dX = dXLocal;
      Notify("GPSOE_Y",dYLocal, moos_time);
      m_dY = dYLocal;
  }

  AppCastingMOOSApp::PostReport();
	return(true);
}

/**
 * \fn
 * \brief Méthode appelée au lancement de l'application
 */

bool GPSoE::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
	setlocale(LC_ALL, "C");

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

	list<string> sParams;
	m_MissionReader.EnableVerbatimQuoting(false);
	if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
	{
		list<string>::iterator p;
		for(p = sParams.begin() ; p != sParams.end() ; p++)
		{
			string orig  = *p;
	    string line  = *p;
	    string param = toupper(biteStringX(line, '='));
	    string value = line;
	    bool handled = false;

			if(param == "ADRESS")
			{
				m_sGpsAddr = value.c_str();
				handled = true;
			}

	    if(!handled)
	      reportUnhandledConfigWarning(orig);
		}
	}

	m_timewarp = GetMOOSTimeWarp();

	registerVariables();
	return(true);
}

void GPSoE::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("POWERED_GPS", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool GPSoE::buildReport()
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
    m_msgs << "iGPSoE Status :                                               \n";
    m_msgs << "==============================================================\n";

    ACTable actab(7);
    actab << "Time | Latitude | Longitude | N | E | X | Y";
    actab.addHeaderLines();
    actab << m_dGpsTime << m_dLatitude << m_dLongitude << m_dN << m_dE << m_dX << m_dY;
    m_msgs << actab.getFormattedString();

  return true;
}
