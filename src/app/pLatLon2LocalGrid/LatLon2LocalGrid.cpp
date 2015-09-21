/************************************************************/
/*    FILE: LatLon2LocalGrid.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "LatLon2LocalGrid.h"

using namespace std;

//---------------------------------------------------------
// Constructor

LatLon2LocalGrid::LatLon2LocalGrid() {
    // Default to the competition site
    latOrigin = 42.954260714055195;
    lonOrigin = 10.601735409849422;

    currentLat=latOrigin;
    currentLon=lonOrigin;

    LAT_SUBSCRIPTION_NAME = "GPS_LAT";
    LON_SUBSCRIPTION_NAME = "GPS_LON";

    // Topics we publish to
    NORTHING_PUBLICATION_NAME = "GPS_N";
    EASTING_PUBLICATION_NAME = "GPS_E";
    geodesy.Initialise(latOrigin, lonOrigin);
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool LatLon2LocalGrid::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();
        
        if (key == LAT_SUBSCRIPTION_NAME) {
            currentLat = msg.GetDouble();
        } else if (key == LON_SUBSCRIPTION_NAME) {
            currentLon = msg.GetDouble();
        } else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }

    return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool LatLon2LocalGrid::OnConnectToServer() {
    registerVariables();
    return true;
}

bool LatLon2LocalGrid::Iterate() {
    AppCastingMOOSApp::Iterate();
    geodesy.LatLong2LocalGrid(currentLat, currentLon, currentNorthing, currentEasting);
    Notify(EASTING_PUBLICATION_NAME, currentEasting);
    Notify(NORTHING_PUBLICATION_NAME, currentNorthing);
    
    AppCastingMOOSApp::PostReport();
    return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LatLon2LocalGrid::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if (!m_MissionReader.GetConfiguration(GetAppName(), sParams))
        reportConfigWarning("No config block found for " + GetAppName());

    STRING_LIST::iterator p;
    sParams.reverse();
    for (p = sParams.begin(); p != sParams.end(); p++) {
        string orig = *p;
        string line = *p;
        string param = toupper(biteStringX(line, '='));
        string value = line;
        bool handled = false;

        if (param == "LatOrigin") {
            latOrigin = atof(value.c_str());
            geodesy.Initialise(latOrigin, lonOrigin);
            handled = true;
        } else if (param == "LongOrigin") {
            lonOrigin = atof(value.c_str());
            geodesy.Initialise(latOrigin, lonOrigin);
            handled = true;
        } else if (param == "NORTHING_PUBLICATION_NAME") {
            NORTHING_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "EASTING_PUBLICATION_NAME") {
            EASTING_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "LAT_SUBSCRIPTION_NAME") {
            LAT_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "LON_SUBSCRIPTION_NAME") {
            LON_SUBSCRIPTION_NAME = value;
            handled = true;
        }
        if (!handled)
            reportUnhandledConfigWarning(orig);
    }

    registerVariables();

    return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void LatLon2LocalGrid::registerVariables() {

    AppCastingMOOSApp::RegisterVariables();
    Register(LON_SUBSCRIPTION_NAME,0);
    Register(LAT_SUBSCRIPTION_NAME,0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool LatLon2LocalGrid::buildReport() {
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

    return true;
}
