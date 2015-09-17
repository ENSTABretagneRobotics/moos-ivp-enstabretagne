/************************************************************/
/*    NAME: Jeremy NICOLA                                   */
/*    ORGN: MIT                                             */
/*    FILE: TrustGPS.cpp                                    */
/*    DATE: 2015                                            */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "TrustGPS.h"

using namespace std;

//---------------------------------------------------------
// Constructor

TrustGPS::TrustGPS() {
    paranoiaLevel = 10;
    this->altitude = -10;
    this->altitude_threshold = 0;
    this->desired_altitude = -10;
    this->desired_altitude_threshold = 0;
    this->gps_quality = false;
    this->paranoiaCounter = 0;
    this->gps_trust = false;
}

bool TrustGPS::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == "GPS_QUALITY") {
            gps_quality = (int) msg.GetDouble();
            
            bool current_gps_trust = gps_quality &&
                    (altitude > altitude_threshold) &&
                    (desired_altitude > desired_altitude_threshold);
            
            memory.push_front(current_gps_trust);
            
            if(current_gps_trust)
            {
                paranoiaCounter++;
            }
            
            if(memory.size()>=paranoiaLevel)
            {
                gps_trust=paranoiaCounter>=paranoiaLevel;
                bool back=memory.back();
                memory.pop_back();
                if(back)
                {
                    paranoiaCounter--;
                }
            }

        } else if (key == "DESIRED_DEPTH") {
            this->desired_altitude = -msg.GetDouble();
        } else if (key == "KELLER_DEPTH") {
            this->altitude = -msg.GetDouble();
        } else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }

    return (true);
}

bool TrustGPS::OnConnectToServer() {
    registerVariables();
    return (true);
}

bool TrustGPS::Iterate() {
    AppCastingMOOSApp::Iterate();
    Notify("GPS_TRUST", gps_trust ? "1" : "0");
    AppCastingMOOSApp::PostReport();
    return (true);
}

bool TrustGPS::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if (!m_MissionReader.GetConfiguration(GetAppName(), sParams))
        reportConfigWarning("No config block found for " + GetAppName());

    STRING_LIST::iterator p;
    for (p = sParams.begin(); p != sParams.end(); p++) {
        string orig = *p;
        string line = *p;
        string param = toupper(biteStringX(line, '='));
        string value = line;

        bool handled = false;
        if (param == "PARANOIA_LEVEL") {
            this->paranoiaLevel = atof(value.c_str());
            handled = true;
        } else if (param == "DESIRED_ALTITUDE_THRESHOLD") {
            this->desired_altitude_threshold = atof(value.c_str());
            handled = true;
        } else if (param == "ALTITUDE_AT_GPS_SUBMERGED") {
            this->altitude_threshold = atof(value.c_str());
            handled = true;
        }
        if (!handled)
            reportUnhandledConfigWarning(orig);
    }

    registerVariables();
    return (true);
}

void TrustGPS::registerVariables() {
    AppCastingMOOSApp::RegisterVariables();
    Register("GPS_QUALITY", 0);
    Register("DESIRED_DEPTH", 0);
    Register("KELLER_DEPTH", 0);
}


bool TrustGPS::buildReport() {
    m_msgs << "============================================ \n";
    m_msgs << "File:                                        \n";
    m_msgs << "============================================ \n";

    ACTable actab(1);
    actab << "GPSTrust";
    actab.addHeaderLines();
    actab << gps_trust;
    m_msgs << actab.getFormattedString();

    return (true);
}




