/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: MapLocalizerIntervals.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "MapLocalizerIntervals.h"
#include <vector>

using namespace std;

MapLocalizerIntervals::MapLocalizerIntervals() {
    qSonar = 10;
    speedNoise = 0.2;
    headingNoise = 2 * M_PI / 180.;
    bufferSize = 20;
    gps_noise = 5;
    initPos = {0, 0};
    filter_easting_initialized = false;
    filter_northing_initialized = false;
    filter_initialized = false;
    speed = 0;
    theta = 0;
    beamRange = 0;
    beamAngle = 0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MapLocalizerIntervals::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == "SPEED_ESTIM_LOCAL") {
            vector<string> std_speed_vect = parseString(msg.getString(), ',');
            speed = atof(std_speed_vect[0].c_str());
            if (filter_initialized) {
                localizer.predict(speed, theta, getMOOSTime());
            }
        } else if (key == "IMU_YAW") {
            theta = msg.getDouble();
        } else if (key == "WALL_DETECTOR") {
            vector<string> std_wall_vect = parseString(msg.getString(), ",");
            beamAngle = atof(parseString(std_wall_vect, "=")[0].c_str());
            beamRange = atof(parseString(std_wall_vect, "=")[1].c_str());

            if (filter_initialized) {
                localizer.update(beamRange, theta + beamAngle, getMOOSTime());
                // Current position with corrections
                // Pour Thomas, quand la boite est vide on la remplace par
                // la valeur de la boite inertielle pure.
                if (localizer.X_cur.is_empty()) {
                    localizer.X_cur[0] = localizer.x_inertial;
                    localizer.X_cur[1] = localizer.y_inertial;
                    localizer.pos[0] = localizer.X_cur;
                }
            }
        } else if (key == "GPS_E") {
            if (gps_trust) {
                gps_easting = msg.getDouble();
                gps_easting_initialized = true;
            }
        } else if (key == "GPS_N") {
            if (gps_trust) {
                gps_northing = msg.getDouble();
                gps_easting_initialized = true;
            }
        } else if (key == "GPS_TRUST") {
            gps_trust = msg.getBool();
            if (gps_trust && gps_easting_initialized && gps_northing_initialized) {
                if (!filter_initialized) {
                    localizer.setInitialPosition(gps_easting, gps_northing, getMOOSTime());
                    filter_initialized = true;
                }
                localizer.setGPSNoise(gps_noise);
                localizer.updateGPS(gps_easting, gps_northing, gps_noise);
            }
        } else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }

    return (true);
}

bool MapLocalizerIntervals::OnConnectToServer() {
    registerVariables();
    return (true);
}

bool MapLocalizerIntervals::Iterate() {
    AppCastingMOOSApp::Iterate();
    stringstream ss;
    ss << filter_initialized << "," << localizer.X_cur[0].lb() << "," << localizer.X_cur[0].ub() << "," << localizer.X_cur[1].lb() << localizer.X_cur[1].ub();
    Notify("POS_ESTIM_INTERVAL", ss.str());
    AppCastingMOOSApp::PostReport();
    return (true);
}

bool MapLocalizerIntervals::OnStartUp() {
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
        if (param == "Q_SONAR") {
            qSonar = (int) atof(value.c_str());
            localizer.setNbOutliers(qSonar);
            handled = true;
        }
        if (param == "MAP_PATH") {
            mapPath = value;
            localizer.setMap(mapPath);
            handled = ture;
        }
        if (param == "SPEED_NOISE") {
            speedNoise = atof(value.c_str());
            localizer.setSpeedNoise(speedNoise);
            handled = true;
        }
        if (param == "HEADING_NOISE") {
            headingNoise = atof(value.c_str());
            localizer.setHeadingNoise(headingNoise);
            handled = true;
        }
        if (param == "GPS_NOISE") {
            gps_noise = atof(value.c_str());
            localizer.setGPSNoise(gps_noise);
            handled = true;
        }
        if (param == "BUFFER_SIZE") {
            buffer_size == (int) atof(value.c_str());
            localizer.setBufferSize(buffer_size);
            handled = true;
        }

        if (!handled)
            reportUnhandledConfigWarning(orig);
    }

    registerVariables();
    return (true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void MapLocalizerIntervals::registerVariables() {
    AppCastingMOOSApp::RegisterVariables();
    Register("SPEED_ESTIM_LOCAL", 0);
    Register("IMU_YAW", 0);
    Register("WALL_DETECTOR", 0);
    Register("GPS_E", 0);
    Register("GPS_N", 0);
    Register("GPS_TRUST", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool MapLocalizerIntervals::buildReport() {
    m_msgs << "============================================ \n";
    m_msgs << "File:                                        \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();

    return (true);
}




