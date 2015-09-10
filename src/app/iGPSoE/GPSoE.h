/**
 * \file GPSoE.h
 * \brief Classe GPSoE
 * \author Team CISSAU - Veni Vidi Vici (ENSTA Bretagne)
 * \version 0.1
 * \date Jun 7th 2013
 *
 * Application MOOS donnant les consignes de déplacement de l'AUV
 *
 */

#ifndef GPSoE_HEADER
#define GPSoE_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"

#include "GPSoEUtils.h"

using namespace std;

class GPSoE : public AppCastingMOOSApp
{
	public:
		GPSoE();
		~GPSoE();

protected: // Standard MOOSApp functions to overload
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload
    void registerVariables();
    bool buildReport();

    inline double DMS2DecDeg(double dfVal)
    {
      int nDeg = (int)(dfVal/100.0);
      double dfTmpDeg = (100.0*(dfVal/100.0-nDeg))/60.0;
      return  dfTmpDeg+nDeg;
    }

	private: // Configuration variables
    std::string   m_sGpsAddr;
    double        m_sLatOrigin;
    double        m_sLongOrigin;

	private: // State variables
		unsigned int  m_iterations;
		double        m_timewarp;

    HGPSOEMGR     hGPSoEMgr;

    double        m_dGpsTime;
    double        m_dLongitude;
    double        m_dLatitude;
    double        m_dN;
    double        m_dE;
    double        m_dX;
    double        m_dY;

    CMOOSGeodesy  m_Geodesy;
};

#endif
