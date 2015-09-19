/************************************************************/
/*    FILE: MixThrusters.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <iterator>
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
    COEFF_MATRIX(0, 1) = 1;
    COEFF_MATRIX(0, 2) = 0;
    COEFF_MATRIX(1, 0) = 1;
    COEFF_MATRIX(1, 1) = -1;
    COEFF_MATRIX(1, 2) = 0;
    COEFF_MATRIX(2, 0) = 0;
    COEFF_MATRIX(2, 1) = 0;
    COEFF_MATRIX(2, 2) = 1;

    desiredForces = Vector3d::Zero();
    u = Vector3d::Zero();
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MixThrusters::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        if (key == "DESIRED_FX") {
            desiredForces[0] = msg.GetDouble();
        } else if (key == "DESIRED_RZ") {
            desiredForces[1] = msg.GetDouble();
        } else if (key == "DESIRED_FZ") {
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

bool MixThrusters::Iterate() {
    AppCastingMOOSApp::Iterate();

    u = COEFF_MATRIX*desiredForces;

    // Saturation
    u.block<2, 1>(0, 0) /= fmax(fabs(u[0]), fabs(u[1]));
    
    Notify(U1_PUBLICATION_NAME,u[0]);
    Notify(U2_PUBLICATION_NAME,u[1]);
    Notify(U3_PUBLICATION_NAME,u[2]);

    AppCastingMOOSApp::PostReport();
    return true;
}

bool MixThrusters::OnStartUp() {
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

        } else if (param == "U1_SUBSCRIPTION_NAME") {
            U1_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "U2_SUBSCRIPTION_NAME") {
            U2_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "U3_SUBSCRIPTION_NAME") {
            U3_SUBSCRIPTION_NAME = value;
            handled = true;
        }else if (param == "U1_PUBLICATION_NAME") {
            U1_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "U2_PUBLICATION_NAME") {
            U2_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "U3_PUBLICATION_NAME") {
            U3_PUBLICATION_NAME = value;
            handled = true;
        }
        if (!handled)
            reportUnhandledConfigWarning(orig);

        registerVariables();
        return true;
    }
}

void MixThrusters::registerVariables() {
    AppCastingMOOSApp::RegisterVariables();
    // Register("FOOBAR", 0);
}

bool MixThrusters::buildReport() {
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
