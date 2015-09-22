/************************************************************/
/*    NAME:                                                 */
/*    ORGN: MIT                                             */
/*    AUTH: Jeremy NICOLA                                   */
/*    FILE: MapLocalizerParticleFilter.cpp                  */
/*    DATE: 2015                                            */
/************************************************************/
#include <stdlib.h>
#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "particle-filter-localization/Eigen/Dense"
#include "MapLocalizerParticleFilter.h"
#include "particle-filter-localization/Eigen/Dense"

using namespace std;
using namespace Eigen;

//---------------------------------------------------------
// Constructor

MapLocalizerParticleFilter::MapLocalizerParticleFilter() {
    this->speedVar = 0.5 * 0.5;
    this->headingVar = 7 * 7;
    this->altitudeVar = 0.15 * 0.15;

    this->gpsEVar = 5 * 5;
    this->gpsNVar = 5 * 5;

    this->beamAngleVar = 7 * 7;
    this->beamRangeVar = 5 * 5;

    this->filter_easting_initialized = false;
    this->filter_northing_initialized = false;
    this->mission_started = false;
    
    lastGPSE = 0;
    lastGPSN = 0;

    lastAltitude = 0;

    lastVelocity = 0;

    lastYaw = 0;
    beamAngle = 0;
    beamRange = 0;

    resampleEvery = 20;

    resamplingMethod = RESIDUAL;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MapLocalizerParticleFilter::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == "WALL_DETECTOR") {
            vector<string> beam = parseString(msg.GetString(), ',');
            beamAngle = atof(parseString(beam[0].c_str(), '=')[1].c_str());
            beamRange = atof(parseString(beam[1].c_str(), '=')[1].c_str());
            // Make sure we are not in
            // the first few frames of the simulation
            if (pf.isInitialized()) {
                pf.update_walls(beamAngle, beamAngleVar, this->lastYaw + beamAngle, headingVar + beamAngleVar);
            }
        }// Receive GPS Easting
        else if (key == "GPS_E") {
            // If we can trust the GPS signal
            if (gps_trust) {
                this->filter_easting_initialized = true;

                // Parse
                this->lastGPSE = msg.GetDouble();

                // If the filter has already been initialized
                // then we update the filter with this position
                if (pf.isInitialized()) {
                    pf.update_GPS(lastGPSE, gpsEVar, lastGPSN, gpsNVar);
                }// If filter not initialized
                    // AND we already have received the northing
                    // then we initialize the filter on this position
                else if (mission_started && filter_northing_initialized) {
                    Vector2d x;
                    x << lastGPSE, lastGPSN;
                    Matrix2d xCov;
                    xCov << gpsEVar, 0, 0, gpsNVar;
                    pf.init(x, xCov);
                }
            }
        }// Receive GPS Northing
        else if (key == "GPS_N") {
            // If we can trust the GPS signal
            if (gps_trust) {
                this->filter_northing_initialized = true;

                // Parse
                this->lastGPSN = msg.GetDouble();
            }
        }// Receive Keller altitude
        else if (key == "KELLER_DEPTH") {
            lastAltitude = -msg.GetDouble();
        }// Receive speed estimate
        else if (key == "SPEED_ESTIM_LOCAL") {
            this->lastVelocity = msg.GetDouble();
            // If the filter has already been initialized
            // then we predict its next state
            if (pf.isInitialized()) {
                Vector2d u(lastVelocity, lastYaw);
                Matrix2d uCov;
                uCov << speedVar, 0, 0, headingVar;
                pf.predict(msg.GetTime(), u, uCov);
            }
        }// Receive IMU YAW
        else if (key == "IMU_YAW") {
            lastYaw = msg.GetDouble();
        } else if (key == "GPS_TRUST") {
            gps_trust = (int) msg.GetDouble();
        } else if(key=="MISSION_STARTED")
        {
            mission_started=true;
        }
        else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }

    return (true);
}

bool MapLocalizerParticleFilter::OnConnectToServer() {
    registerVariables();
    return (true);
}

bool MapLocalizerParticleFilter::Iterate() {
    AppCastingMOOSApp::Iterate();
    if (pf.isInitialized()) {
        Vector2d pos = pf.computeMean();
        Matrix2d posCov = pf.computeCovariance();
        Vector2d posWeighted = pf.computeWeightedMean();
        Matrix2d posCovWeighted = pf.computeWeightedCovariance();

        stringstream ss;
        ss << pos(0) << "," << pos(1);
        Notify("POS_ESTIM_PARTICLE_FILTER", ss.str().c_str());

        ss.clear();
        ss.str(std::string());

        ss << posCov(0, 0) << "," << posCov(0, 1) << posCov(1, 0) << "," << posCov(1, 1);
        Notify("POS_COV_ESTIM_PARTICLE_FILTER", ss.str().c_str());

        ss.clear();
        ss.str(std::string());

        ss << posWeighted(0) << "," << posWeighted(1);
        Notify("POS_ESTIM_PARTICLE_FILTER_WEIGHTED", ss.str().c_str());

        ss.clear();
        ss.str(std::string());

        ss << posCovWeighted(0, 0) << "," << posCovWeighted(0, 1) << posCovWeighted(1, 0) << "," << posCovWeighted(1, 1);
        Notify("POS_COV_ESTIM_PARTICLE_FILTER_WEIGHTED", ss.str().c_str());
    }
    AppCastingMOOSApp::PostReport();
    return (true);
}

bool MapLocalizerParticleFilter::OnStartUp() {
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
        if (param == "MAP_PATH") {
            pf.setWallsFile(value);
            handled = true;
        } else if (param == "SPEED_VAR") {
            this->speedVar = atof(value.c_str());
            handled = true;
        } else if (param == "HEADING_VAR") {
            this->headingVar = atof(value.c_str());
            handled = true;
        } else if (param == "ALTITUDE_VAR") {
            this->altitudeVar = atof(value.c_str());
            handled = true;
        } else if (param == "GPS_E_VAR") {
            this->gpsEVar = atof(value.c_str());
            handled = true;
        } else if (param == "GPS_N_VAR") {
            this->gpsNVar = atof(value.c_str());
            handled = true;
        } else if (param == "BEAM_ANGLE_VAR") {
            this->beamAngleVar = atof(value.c_str());
            handled = true;
        } else if (param == "BEAM_RANGE_VAR") {
            this->beamRangeVar = atof(value.c_str());
            handled = true;
        } else if (param == "RESAMPLE_EVERY") {
            this->resampleEvery = (int) atof(value.c_str());
            handled = true;
        } else if (param == "RESAMPLING_METHOD") {
            if (value == "MULTINOMIAL") {
                resamplingMethod = MULTINOMIAL;
            } else if (value == "LOW_VARIANCE") {
                resamplingMethod = LOW_VARIANCE;
            } else if (value == "RESIDUAL") {
                resamplingMethod = RESIDUAL;
            }
            handled = true;
        }

        if (!handled)
            reportUnhandledConfigWarning(orig);

    }

    registerVariables();
    return (true);
}

void MapLocalizerParticleFilter::registerVariables() {
    AppCastingMOOSApp::RegisterVariables();
    Register("WALL_DETECTOR", 0);
    Register("GPS_E", 0);
    Register("GPS_N", 0);
    Register("TRUST_GPS", 0);
    Register("KELLER_DEPTH", 0); // iKeller
    Register("SPEED_ESTIM_LOCAL", 0);
    Register("IMU_YAW", 0);
    Register("MISSION_STARTED",0);
}

bool MapLocalizerParticleFilter::buildReport() {
    ACTable actab(4);
    actab << "Nb. Particles | SpeedVar | HeadingVar | Alt. Var ";
    actab.addHeaderLines();
    actab << PARTICLE_NUMBER << speedVar << headingVar << altitudeVar;
    m_msgs << actab.getFormattedString();

    actab = ACTable(4);
    actab << "GPSEVar | GPSNVar | Beam Angle Var. | Beam Range Var";
    actab.addHeaderLines();
    actab << gpsEVar << gpsNVar << beamAngleVar << beamRangeVar;
    m_msgs << actab.getFormattedString();

    actab = ACTable(2);
    actab << " Resample every | Resampling Method";
    actab.addHeaderLines();
    string s_method = "?????";
    if (resamplingMethod == MULTINOMIAL) {
        s_method = "MULTINOMIAL";
    } else if (resamplingMethod == LOW_VARIANCE) {
        s_method = "LOW_VARIANCE";
    } else if (resamplingMethod == RESIDUAL) {
        s_method = "RESIDUAL";
    }
    actab << resampleEvery << s_method;
    m_msgs << actab.getFormattedString();
    
    return (true);
}
