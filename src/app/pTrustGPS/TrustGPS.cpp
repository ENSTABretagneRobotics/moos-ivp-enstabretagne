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
ostringstream oss;

//---------------------------------------------------------
// Constructor

TrustGPS::TrustGPS() {
    bufferLevel = 10;
    this->altitude = 0.1;
    this->altitude_threshold = 0;
    this->gps_quality = false;
    this->gps_fix = 0;
    this->bufferCounter = 0;
    this->gps_trust = false;

    // Topics we subscribe to
    GPS_FIX_SUBSCRIPTION_NAME = "GPS_FIX";
    GPS_SIG_SUBSCRIPTION_NAME = "GPS_SIG";
    KELLER_DEPTH_SUBSCRIPTION_NAME = "KELLER_DEPTH";

    // Topics we publish to
    GPS_TRUST_PUBLICATION_NAME = "GPS_TRUST";
}

bool TrustGPS::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);
    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == GPS_SIG_SUBSCRIPTION_NAME) {
            gps_quality = (int) msg.GetDouble();

            bool current_gps_trust = gps_quality >= 1 &&
                    (altitude > altitude_threshold) &&
                    (gps_fix >= 2);

            memory.push_front(current_gps_trust);

            if (current_gps_trust) {
                bufferCounter++;
            }

            if (memory.size() > bufferLevel) {
                bool back = memory.back();
                memory.pop_back();
                if (back) {
                    bufferCounter--;
                }
                gps_trust = bufferCounter >= bufferLevel;
                //bufferCounter=max(0,bufferCounter);
            }

        } else if (key == GPS_FIX_SUBSCRIPTION_NAME) {
            this->gps_fix = msg.GetDouble();
        } else if (key == KELLER_DEPTH_SUBSCRIPTION_NAME) {
            this->altitude = -fabs(msg.GetDouble());
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
    Notify(GPS_TRUST_PUBLICATION_NAME, gps_trust ? 1: 0);
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
        if (param == "BUFFER_SIZE") {
            this->bufferLevel = atof(value.c_str());
            handled = true;
        } else if (param == "ALTITUDE_THRESHOLD") {
            this->altitude_threshold = atof(value.c_str());
            handled = true;
        } else if (param == "GPS_FIX_SUBSCRIPTION_NAME") {
            this->GPS_FIX_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "GPS_SIG_SUBSCRIPTION_NAME") {
            this->GPS_SIG_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "KELLER_DEPTH_SUBSCRIPTION_NAME") {
            this->KELLER_DEPTH_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "GPS_TRUST_PUBLICATION_NAME") {
            this->GPS_TRUST_PUBLICATION_NAME = value;
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
    Register(GPS_SIG_SUBSCRIPTION_NAME, 0);
    Register(GPS_FIX_SUBSCRIPTION_NAME, 0);
    Register(KELLER_DEPTH_SUBSCRIPTION_NAME, 0);
}

bool TrustGPS::buildReport() {
    ACTable actab(1);
    actab << "GPSTrust";
    actab.addHeaderLines();
    actab << gps_trust;
    m_msgs << actab.getFormattedString();

    return (true);
}




