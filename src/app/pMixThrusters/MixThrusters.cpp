/************************************************************/
/*    FILE: MixThrusters.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include <list>
#include "MBUtils.h"
#include "ACTable.h"
#include "MixThrusters.h"
#include "Eigen/Dense"

using namespace std;
using namespace Eigen;

//---------------------------------------------------------
// Constructor

MixThrusters::MixThrusters() {
    // Wild guesses until we receive
    // the configuration
    COEFF_MATRIX(0, 0) = 1;
    COEFF_MATRIX(0, 1) = -1;
    COEFF_MATRIX(0, 2) = 0;
    COEFF_MATRIX(1, 0) = 1;
    COEFF_MATRIX(1, 1) = 1;
    COEFF_MATRIX(1, 2) = 0;
    COEFF_MATRIX(2, 0) = 0;
    COEFF_MATRIX(2, 1) = 0;
    COEFF_MATRIX(2, 2) = 1;

    desiredForces = Vector3d::Zero();
    u = Vector3d::Zero();

    this->saturation_mod = NORMALIZATION;

    m_forward_coeff = 1.0;
    m_backward_coeff = 1.0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MixThrusters::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == FX_SUBSCRIPTION_NAME) {
            desiredForces[0] = msg.GetDouble();
        } else if (key == RZ_SUBSCRIPTION_NAME) {
            desiredForces[1] = msg.GetDouble();
        } else if (key == FZ_SUBSCRIPTION_NAME) {
            desiredForces[2] = msg.GetDouble();
        } else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }
    return true;
}

bool MixThrusters::OnConnectToServer() {
    registerVariables();
    return true;
}

void MixThrusters::saturationSigmoid(Eigen::Vector3d &u) {

    double S0 = 2.0 / (1.0 + exp(-u[0] / m_sigmoid_coeff)) - 1.0;
    double S1 = 2.0 / (1.0 + exp(-u[1] / m_sigmoid_coeff)) - 1.0;

    double maxU = fmax(fabs(S0), fabs(S1));

    if(maxU!=0){
        u[0] = fsign(u[0]) * S0/maxU;
        u[1] = fsign(u[1]) * S1/maxU;
    }
    else{
        u[0]=0.0;
        u[1]=0.0;
    }

    // Normalize so that u3 is in [-1,1]
    if (fabs(u[2]) > 1) {
        u[2] /= fabs(u[2]);
    }
}

double MixThrusters::fsign(double val){
    if(val>=0)
        return 1.0;
    else
        return -1.0;
}

void MixThrusters::saturationNormalization(Eigen::Vector3d &u) {
    // Saturation
    if (fmax(fabs(u[0]), fabs(u[1])) > 1) {
        u.block<2, 1>(0, 0) /= fmax(fabs(u[0]), fabs(u[1]));
    }

    // Normalize so that u3 is in [-1,1]
    if (fabs(u[2]) > 1) {
        u[2] /= fabs(u[2]);
    }
}

bool MixThrusters::Iterate() {
    AppCastingMOOSApp::Iterate();

    u = COEFF_MATRIX*desiredForces;

    u[0] = sensCorrection(u[0]);
    u[1] = sensCorrection(u[1]);

    switch (saturation_mod) {
        case NORMALIZATION:
            saturationNormalization(u);
            break;
        case SIGMOID:
            saturationSigmoid(u);
            break;
        default:
            // Shouldn't happen, but...
            saturationNormalization(u);
    }

    Notify(U1_PUBLICATION_NAME, u[0]);
    Notify(U2_PUBLICATION_NAME, u[1]);
    Notify(U3_PUBLICATION_NAME, u[2]);

    AppCastingMOOSApp::PostReport();
    return true;
}

bool MixThrusters::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    string res;
    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if (!m_MissionReader.GetConfiguration(GetAppName(), sParams))
        reportConfigWarning("No config block found for " + GetAppName());
    if (!m_MissionReader.GetValue("COEFF_MATRIX", res)) {
        reportConfigWarning("No COEFF_MATRIX config found for " + GetAppName());
    }

    STRING_LIST::iterator p;
    sParams.reverse();
    for (p = sParams.begin(); p != sParams.end(); p++) {
        string orig = *p;
        string line = *p;
        string param = toupper(biteStringX(line, '='));
        string value = line;
        bool handled = false;

        if (param == "COEFF_MATRIX") {
            vector<string> str_vector = parseString(value, ',');

            //check for correct array size of 9 elements (3x3)
            if (str_vector.size() == 9) {
                unsigned int i = 0;
                for (unsigned int j = 0; j < 3; j++) {
                    for (unsigned int k = 0; k < 3; k++) {
                        COEFF_MATRIX(j, k) = atof(str_vector[i].c_str());
                        i++;
                    }
                }
            } else {
                //incorrect array size
                //send warning
                reportUnhandledConfigWarning("Error while parsing COEFF_MATRIX: incorrect number of elements");
            }//end of else
            handled = true;
        } else if (param == "SATURATION_MOD") {
            if (value == "NORMALIZATION") {
                this->saturation_mod = NORMALIZATION;
                handled = true;
            } else if (value == "SIGMOID") {
                this->saturation_mod = SIGMOID;
                handled = true;
            }
        } else if (param == "FX_SUBSCRIPTION_NAME") {
            FX_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "RZ_SUBSCRIPTION_NAME") {
            RZ_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "FZ_SUBSCRIPTION_NAME") {
            FZ_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "U1_PUBLICATION_NAME") {
            U1_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "U2_PUBLICATION_NAME") {
            U2_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "U3_PUBLICATION_NAME") {
            U3_PUBLICATION_NAME = value;
            handled = true;
        }else if (param == "FORWARD_COEFF") {
            m_forward_coeff = atof(value.c_str());
            handled = true;
        } else if (param == "BACKWARD_COEFF") {
            m_backward_coeff = atof(value.c_str());
            handled = true;
        } else if (param == "SIGMOID_COEFF") {
            m_sigmoid_coeff = atof(value.c_str());
            handled = true;
        }

        if (!handled)
            reportUnhandledConfigWarning(orig);

    }
    registerVariables();
    return true;
}

void MixThrusters::registerVariables() {
    AppCastingMOOSApp::RegisterVariables();
    Register(FX_SUBSCRIPTION_NAME, 0);
    Register(RZ_SUBSCRIPTION_NAME, 0);
    Register(FZ_SUBSCRIPTION_NAME, 0);
}

bool MixThrusters::buildReport() {
#if 0 // Keep these around just for template
    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
#endif

    ACTable actab(3);
    actab << "Left | Right | Up";
    actab.addHeaderLines();
    actab << u[0] << u[1] << u[2];
    m_msgs << actab.getFormattedString();

    return true;
}

double MixThrusters::sensCorrection(double val){
    if(val>0)
        return val * m_forward_coeff;
    else
        return val * m_forward_coeff;
}
