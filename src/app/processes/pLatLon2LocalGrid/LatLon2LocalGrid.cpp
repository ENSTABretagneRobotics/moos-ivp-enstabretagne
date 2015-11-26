/************************************************************/
/*    FILE: LatLon2LocalGrid.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <sstream>
#include <iomanip>
#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "LatLon2LocalGrid.h"

using namespace std;

//---------------------------------------------------------
// Constructor

LatLon2LocalGrid::LatLon2LocalGrid() {
    
    
    // Default to the competition site
    lat_origin = 0.;
    long_origin = 0.;

    m_custom_shift_x = 0.;
    m_custom_shift_y = 0.;

    currentLat=lat_origin;
    currentLon=long_origin;

    LAT_SUBSCRIPTION_NAME = "GPS_LAT";
    LON_SUBSCRIPTION_NAME = "GPS_LON";

    // Topics we publish to
    NORTHING_PUBLICATION_NAME = "GPS_N";
    EASTING_PUBLICATION_NAME = "GPS_E";
    geodesy.Initialise(lat_origin, long_origin);
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
    Notify(EASTING_PUBLICATION_NAME, currentEasting + m_custom_shift_x);
    Notify(NORTHING_PUBLICATION_NAME, currentNorthing + m_custom_shift_y);
    
    AppCastingMOOSApp::PostReport();
    return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LatLon2LocalGrid::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    bool geodesy_origin_param = true;

    if(!m_MissionReader.GetValue("LatOrigin", lat_origin))
    {
      reportConfigWarning("No LatOrigin in *.moos file");
      geodesy_origin_param = false;
    }

    if(!m_MissionReader.GetValue("LongOrigin", long_origin))
    {
      reportConfigWarning("No LongOrigin in *.moos file");
      geodesy_origin_param = false;
    }

    if(geodesy_origin_param)
        geodesy.Initialise(lat_origin, long_origin);

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

        if (param == "NORTHING_PUBLICATION_NAME") {
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
        } else if (param == "CUSTOM_SHIFT_X") {
            m_custom_shift_x = atof(value.c_str());
            handled = true;
        } else if (param == "CUSTOM_SHIFT_Y") {
            m_custom_shift_y = atof(value.c_str());
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

    ostringstream strs;
    strs << lat_origin;
    string lat_str = strs.str();
    strs.str("");
    strs.clear();
    strs << long_origin;
    string long_str = strs.str();

    ACTable actab_geo(2);
    actab_geo << "Lat origin | Long origin";
    actab_geo.addHeaderLines();
    actab_geo << lat_str << long_str;
    m_msgs << actab_geo.getFormattedString() << endl << endl;

    ACTable actab_variables(3);
    actab_variables << "From | | To";
    actab_variables.addHeaderLines();
    actab_variables << LAT_SUBSCRIPTION_NAME << "-> " << NORTHING_PUBLICATION_NAME;
    actab_variables << LON_SUBSCRIPTION_NAME << "-> " << EASTING_PUBLICATION_NAME;
    m_msgs << actab_variables.getFormattedString() << endl << endl;

    ACTable actab_shift(2);
    actab_shift << "Shift X | Shift Y";
    actab_shift.addHeaderLines();
    actab_shift << m_custom_shift_x << m_custom_shift_y;
    m_msgs << actab_shift.getFormattedString();

    return true;
}
