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
    this->speedVar = 0.5 * 0.5; // 0.5 m of std. Dev
    this->headingVar = 7 * 7; // 7 Degrees of std. Dev
    this->altitudeVar = 0.15 * 0.15; // 15cm of std. Dev

    this->gpsEVar = 5 * 5; // 5 meters of std. Dev
    this->gpsNVar = 5 * 5; // 5 meters of std. Dev

    this->beamAngleVar = 7 * 7; // 7 degrees of std. Dev for the beam angle
    this->beamRangeVar = 5 * 5; // 5 meters of std. Dev for the beam range
    this->receivedBeam = false;

    this->gps_trust = false;
    this->filter_easting_initialized = false;
    this->filter_northing_initialized = false;

    lastGPSE = 0;
    lastGPSN = 0;

    lastAltitude = 0;

    lastVelocity = 0;

    lastYaw = 0;
    beamAngle = 0;
    beamRange = 0;

    resampleEvery = 20;

    resamplingMethod = RESIDUAL;

    RESET_SUBSCRIPTION_NAME = "RESET";

    POS_X_PUBLICATION_NAME = "POS_X_ESTIM_PF";
    POS_X_WEIGHTED_PUBLICATION_NAME = "POS_Y_ESTIM_PF";
    POS_X_PUBLICATION_NAME = "POS_X_ESTIM_PF_WEIGHTED";
    POS_Y_WEIGHTED_PUBLICATION_NAME = "POS_Y_ESTIM_PF_WEIGHTED";

    POS_COV_XX_PUBLICATION_NAME = "POS_COV_XX_PF";
    POS_COV_XY_PUBLICATION_NAME = "POS_COV_XY_PF";
    POS_COV_YY_PUBLICATION_NAME = "POS_COV_YY_PF";

    POS_COV_XX_WEIGHTED_PUBLICATION_NAME = "POS_COV_XX_WEIGHTED_PF";
    POS_COV_XY_WEIGHTED_PUBLICATION_NAME = "POS_COV_XY_WEIGHTED_PF";
    POS_COV_YY_WEIGHTED_PUBLICATION_NAME = "POS_COV_YY_WEIGHTED_PF";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MapLocalizerParticleFilter::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == WALL_DETECTOR_SUBSCRIPTION_NAME) {
            vector<string> beam = parseString(msg.GetString(), ',');
            beamAngle = atof(parseString(beam[0].c_str(), '=')[1].c_str()); // Radians, sens horaire, 0 au nord, reference geographiquement
            beamRange = atof(parseString(beam[1].c_str(), '=')[1].c_str());

            receivedBeam = true;
        }// Receive GPS Easting
        else if (key == GPS_E_SUBSCRIPTION_NAME) {
            // If we can trust the GPS signal
            if (gps_trust) {
                this->filter_easting_initialized = true;
                // Parse
                this->lastGPSE = msg.GetDouble();
            }
        }// Receive GPS Northing
        else if (key == GPS_N_SUBSCRIPTION_NAME) {
            // If we can trust the GPS signal
            if (gps_trust) {
                this->filter_northing_initialized = true;

                // Parse
                this->lastGPSN = msg.GetDouble();
            }
        }// Receive Keller altitude
        else if (key == KELLER_DEPTH_SUBSCRIPTION_NAME) {
            lastAltitude = -fabs(msg.GetDouble());
        }// Receive speed estimate
        else if (key == SPEED_ESTIM_LOCAL_SUBSCRIPTION_NAME) {

            cout << "RECEIVED SPEED_ESTIM_LOCAL_SUBSCRIPTION_NAME STRING: " << msg.GetString() << endl;
            cout << "RECEIVED SPEED_ESTIM_LOCAL_SUBSCRIPTION_NAME DOUBLE: " << msg.GetDouble() << endl;
            this->lastVelocity = msg.GetDouble();
        }// Receive IMU YAW
        else if (key == IMU_YAW_SUBSCRIPTION_NAME) {
            lastYaw = msg.GetDouble();
        } else if (key == GPS_TRUST_SUBSCRIPTION_NAME) {
            gps_trust = (int) msg.GetDouble();
        } else if (key == RESET_SUBSCRIPTION_NAME) {
            Vector2d x;
            x << lastGPSE, lastGPSN;
            Matrix2d xCov;
            xCov << gpsEVar, 0, 0, gpsNVar;
            pf.init(x, xCov);

        } else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
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

        Vector2d mean = pf.computeMean();

        // If the filter has already been initialized
        // then we update the filter with this position
        if (gps_trust) {
            //cout << "WE UPDATE GPS"<<endl;
            //cout << "LastGPSE: "<<lastGPSE<<endl;
            //cout << "LastGPSN: "<<lastGPSN<<endl;
            //cout << "meanX: "<<mean[0]<<endl;
            //cout << "meanY: "<<mean[1]<<endl;
            pf.update_GPS(lastGPSE, gpsEVar, lastGPSN, gpsNVar);
        }

        if (receivedBeam) {
            //cout << "We update range: " << beamRange << endl;
            //cout << "lastYaw: " << lastYaw << endl;
            //cout << "beamAngle: " << beamAngle << endl;
            pf.update_walls(beamRange, beamRangeVar, this->lastYaw + beamAngle, headingVar + beamAngleVar);
            receivedBeam = false;
        }

        // If the filter has already been initialized
        // then we predict its next state
        //lastYaw *= M_PI / 180.0;
        Vector2d u(lastVelocity, lastYaw);
        Matrix2d uCov;

        cout << "lastYaw: " << endl << lastYaw << endl;
        cout << "lastVelocity: " << endl << lastVelocity << endl;

        uCov << speedVar, 0, 0, headingVar;
        pf.predict(MOOSTime(), u, uCov);

        Vector2d pos = pf.computeMean();

        Notify(POS_X_PUBLICATION_NAME, pos[0]);
        Notify(POS_Y_PUBLICATION_NAME, pos[1]);

        Matrix2d posCov = pf.computeCovariance();

        Notify(POS_COV_XX_PUBLICATION_NAME, posCov(0, 0));
        Notify(POS_COV_XY_PUBLICATION_NAME, posCov(0, 1));
        Notify(POS_COV_YY_PUBLICATION_NAME, posCov(1, 1));

        Vector2d posWeighted = pf.computeWeightedMean();

        Notify(POS_X_WEIGHTED_PUBLICATION_NAME, posWeighted[0]);
        Notify(POS_Y_WEIGHTED_PUBLICATION_NAME, posWeighted[1]);

        Matrix2d posCovWeighted = pf.computeWeightedCovariance();

        Notify(POS_COV_XX_WEIGHTED_PUBLICATION_NAME, posCovWeighted(0, 0));
        Notify(POS_COV_XY_WEIGHTED_PUBLICATION_NAME, posCovWeighted(0, 1));
        Notify(POS_COV_YY_WEIGHTED_PUBLICATION_NAME, posCovWeighted(1, 1));
    } else if (gps_trust && filter_easting_initialized && filter_northing_initialized) {
        Vector2d x;
        x << lastGPSE, lastGPSN;
        Matrix2d xCov;
        xCov << gpsEVar, 0, 0, gpsNVar;

        pf.init(x, xCov);
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
                pf.setResampleMethod(resamplingMethod);
            } else if (value == "LOW_VARIANCE") {
                resamplingMethod = LOW_VARIANCE;
                pf.setResampleMethod(resamplingMethod);
            } else if (value == "RESIDUAL") {
                resamplingMethod = RESIDUAL;
                pf.setResampleMethod(resamplingMethod);
            }
            handled = true;
        }// Topics we subscribe to
        else if (param == "RESET_SUBSCRIPTION_NAME") {
            RESET_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "WALL_DETECTOR_SUBSCRIPTION_NAME") {
            WALL_DETECTOR_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "GPS_E_SUBSCRIPTION_NAME") {
            GPS_E_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "GPS_N_SUBSCRIPTION_NAME") {
            GPS_N_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "KELLER_DEPTH_SUBSCRIPTION_NAME") {
            KELLER_DEPTH_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "SPEED_ESTIM_LOCAL_SUBSCRIPTION_NAME") {
            SPEED_ESTIM_LOCAL_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "IMU_YAW_SUBSCRIPTION_NAME") {
            IMU_YAW_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "GPS_TRUST_SUBSCRIPTION_NAME") {
            GPS_TRUST_SUBSCRIPTION_NAME = value;
            handled = true;
        }// Topice we publish to
        else if (param == "POS_X_PUBLICATION_NAME") {
            POS_X_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_Y_PUBLICATION_NAME") {
            POS_Y_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_COV_XX_PUBLICATION_NAME") {
            POS_COV_XX_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_COV_XY_PUBLICATION_NAME") {
            POS_COV_XY_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_COV_YY_PUBLICATION_NAME") {
            POS_COV_YY_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_X_WEIGHTED_PUBLICATION_NAME") {
            POS_X_WEIGHTED_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_Y_WEIGHTED_PUBLICATION_NAME") {
            POS_Y_WEIGHTED_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_COV_XX_WEIGHTED_PUBLICATION_NAME") {
            POS_COV_XX_WEIGHTED_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_COV_XY_WEIGHTED_PUBLICATION_NAME") {
            POS_COV_XY_WEIGHTED_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_COV_YY_WEIGHTED_PUBLICATION_NAME") {
            POS_COV_YY_WEIGHTED_PUBLICATION_NAME = value;
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
    Register(WALL_DETECTOR_SUBSCRIPTION_NAME, 0);
    Register(GPS_E_SUBSCRIPTION_NAME, 0);
    Register(GPS_N_SUBSCRIPTION_NAME, 0);
    Register(GPS_TRUST_SUBSCRIPTION_NAME, 0);
    Register(KELLER_DEPTH_SUBSCRIPTION_NAME, 0); // iKeller
    Register(SPEED_ESTIM_LOCAL_SUBSCRIPTION_NAME, 0);
    Register(IMU_YAW_SUBSCRIPTION_NAME, 0);
    Register(RESET_SUBSCRIPTION_NAME, 0);
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
