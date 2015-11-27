/************************************************************/
/*    NAME: Jeremy NICOLA                                              */
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne                                             */
/*    FILE: EstimSpeed.cpp                                        */
/*    DATE: 2015                                                */
/************************************************************/

#include <iterator>
#include <math.h>
#include "MBUtils.h"
#include "ACTable.h"
#include "EstimSpeed.h"
#include "Eigen/Dense"
#include <cmath>

using namespace std;
using namespace Eigen;

//---------------------------------------------------------
// Constructor

EstimSpeed::EstimSpeed() {
    // INITIALIZE VARIABLES


    /*
    default values for COEFF_MATRIX

       Fx   Fr   Fz
      ---------------
  u1 |  1   1   0 
  u2 |  1   -1  0
  u3 |  0   0   1   

  COEFF_MATRIX = { 
    {  1 ,  1 , 0  }, 
    {  1 ,  1 , 0  }, 
    {  0,   0 , 1  }
  };
     */
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

    COEFF_MATRIX_INV = COEFF_MATRIX.inverse();

    DAMPING_MATRIX = Matrix3d::Identity()*0.4;

    MASS = 20;

    imu_yaw = 0;
    u = Vector3d::Zero();
    rot = Matrix3d::Identity();

    X = Vector3d::Zero();
    V = Vector3d::Zero();
    v = Vector3d::Zero();
}

bool EstimSpeed::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();
       if (key == YAW_REGISTRATION_NAME) {
            this->imu_yaw = msg.GetDouble();
        }// We receive LEFT thruster value, in [-1,1]]
        else if (key == U1_SUBSCRIPTION_NAME) {
            this->u[0] = msg.GetDouble();
        }// We receive RIGHT thruster value, in [-1,1]]
        else if (key == U2_SUBSCRIPTION_NAME) {
            this->u[1] = msg.GetDouble();
        }// We receive VERTICAL thruster value, in [-1,1]]
        else if (key == U3_SUBSCRIPTION_NAME) {
            this->u[2] = msg.GetDouble();
        } else if (key == GPS_E_SUBSCRIPTION_NAME) {
            X_GPS_PRESSURE[0] = msg.GetDouble();
        } else if (key == GPS_N_SUBSCRIPTION_NAME) {
            X_GPS_PRESSURE[1] = msg.GetDouble();
        } else if (key == KELLER_DEPTH_SUBSCRIPTION_NAME) {
            X_GPS_PRESSURE[2] = msg.GetDouble();
        } else if (key == RESET_SUBSCRIPTION_NAME) {
            X = X_GPS_PRESSURE;
            V = Vector3d::Zero();
            v = Vector3d::Zero();
        } else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }
    return (true);
}

bool EstimSpeed::OnConnectToServer() {
    registerVariables();
    return (true);
}

bool EstimSpeed::Iterate() {
    AppCastingMOOSApp::Iterate();
    
    double cTheta = cos(imu_yaw);
    double sTheta = sin(imu_yaw);
    rot.block<2, 2>(0, 0) = (Matrix2d() << cTheta, -sTheta, sTheta, cTheta).finished();

    // The Dead-reckoning is here
    double delta_t = MOOSTime() - old_MOOSTime;
    old_MOOSTime = MOOSTime();

    X += delta_t*V;
    Vector3d vS;

    vS << v[0] * abs(v[0]), v[1] * abs(v[1]), v[2] * abs(v[2]);

    Vector3d sumForcesLocal = (-10 * DAMPING_MATRIX * vS + COEFF_MATRIX_INV * u);
    Vector3d sumForcesGlobal = rot * sumForcesLocal;

    V += delta_t * sumForcesGlobal / MASS;
    v = rot.transpose() * V;

    stringstream ss;

    ss << X[0] << "," << X[1] << "," << X[2];
    Notify(POS_DEAD_RECKONING_PUBLICATION_NAME, ss.str());

    
    
    ss.clear();
    ss.str(std::string());

    ss << v[0] << "," << v[1] << "," << v[2];
    Notify(SPEED_LOCAL_DEAD_RECKONING_PUBLICATION_NAME, v[0]);//ss.str());

    Notify(POS_DEAD_RECKONING_E_PUBLICATION_NAME, X[0]);
    Notify(POS_DEAD_RECKONING_N_PUBLICATION_NAME, X[1]);
    Notify(POS_DEAD_RECKONING_U_PUBLICATION_NAME, X[2]);

    ss.clear();
    ss.str(std::string());

    ss << V[0] << "," << V[1] << "," << V[2];
    Notify(SPEED_GLOBAL_DEAD_RECKONING_PUBLICATION_NAME, ss.str());

    ss.str(std::string());

    AppCastingMOOSApp::PostReport();
    return (true);
}

bool EstimSpeed::OnStartUp() {
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

        if (param == "COEFF_MATRIX") {
            vector<string> str_vector = parseString(value, ',');

            //check for correct array size of 9 elements (3x3)
            if (str_vector.size() == 9) {
                unsigned int i = 0;
                for (unsigned int j = 0; j < 3; j++) {
                    for (unsigned int k = 0; k < 3; k++) {
                        //get the string and convert it into value
                        COEFF_MATRIX(j, k) = atof(str_vector[i].c_str());
                        i++;
                    }
                }
                COEFF_MATRIX_INV = COEFF_MATRIX.inverse();
            } else {
                //incorrect array size
                //send warning
                reportUnhandledConfigWarning("Error while parsing COEFF_MATRIX: incorrect number of elements");

            }//end of else

            handled = true;
        } else if (param == "DAMPING_MATRIX") {
            vector<string> str_vector = parseString(value, ',');

            //check for correct array size of 9 elements (3x3)
            if (str_vector.size() == 9) {
                unsigned int i = 0;
                for (unsigned int j = 0; j < 3; j++) {
                    for (unsigned int k = 0; k < 3; k++) {
                        //get the string and convert it into value
                        DAMPING_MATRIX(j, k) = atof(str_vector[i].c_str());
                        i++;
                    }
                }
            } else {
                //incorrect array size
                //send warning
                reportUnhandledConfigWarning("Error while parsing DAMPING_MATRIX: incorrect number of elements");
            }//end of else
            handled = true;
        } else if (param == "MASS") {
            this->MASS = atof(value.c_str());
            handled = true;
        } else if (param == "YAW_REGISTRATION_NAME") {
            this->YAW_REGISTRATION_NAME = value;
            handled = true;
        } else if (param == "U1_SUBSCRIPTION_NAME") {
            this->U1_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "U2_SUBSCRIPTION_NAME") {
            this->U2_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "U3_SUBSCRIPTION_NAME") {
            this->U3_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "POS_DEAD_RECKONING_PUBLICATION_NAME") {
            this->POS_DEAD_RECKONING_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "SPEED_LOCAL_DEAD_RECKONING_PUBLICATION_NAME") {
            this->SPEED_LOCAL_DEAD_RECKONING_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "SPEED_GLOBAL_DEAD_RECKONING_PUBLICATION_NAME") {
            this->SPEED_GLOBAL_DEAD_RECKONING_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "RESET_SUBSCRIPTION_NAME") {
            this->RESET_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "GPS_E_SUBSCRIPTION_NAME") {
            this->GPS_E_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "GPS_N_SUBSCRIPTION_NAME") {
            this->GPS_N_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "KELLER_DEPTH_SUBSCRIPTION_NAME") {
            this->KELLER_DEPTH_SUBSCRIPTION_NAME = value;
            handled = true;
        } else if (param == "POS_DEAD_RECKONING_E_PUBLICATION_NAME") {
            this->POS_DEAD_RECKONING_E_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_DEAD_RECKONING_N_PUBLICATION_NAME") {
            this->POS_DEAD_RECKONING_N_PUBLICATION_NAME = value;
            handled = true;
        } else if (param == "POS_DEAD_RECKONING_U_PUBLICATION_NAME") {
            this->POS_DEAD_RECKONING_U_PUBLICATION_NAME = value;
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

void EstimSpeed::registerVariables() {

    AppCastingMOOSApp::RegisterVariables();
    Register(YAW_REGISTRATION_NAME, 0);
    Register(U1_SUBSCRIPTION_NAME, 0);
    Register(U2_SUBSCRIPTION_NAME, 0);
    Register(U3_SUBSCRIPTION_NAME, 0);
    Register(RESET_SUBSCRIPTION_NAME, 0);
    Register(GPS_E_SUBSCRIPTION_NAME, 0);
    Register(GPS_N_SUBSCRIPTION_NAME, 0);
    Register(KELLER_DEPTH_SUBSCRIPTION_NAME, 0);
}

bool EstimSpeed::buildReport() {
    m_msgs << "============================================ \n";
    m_msgs << "pEstimSpeed                                  \n";
    m_msgs << "============================================ \n";
    m_msgs << "\n";

    ACTable actab(2);
    actab << "Yaw | Mass ";
    actab.addHeaderLines();
    actab << imu_yaw << MASS;
    m_msgs << actab.getFormattedString();

    actab = ACTable(4);
    actab << " . | X | V | v";
    actab.addHeaderLines();
    actab << "(0)" << X[0] << V[0] << v[0];
    actab << "(1)" << X[1] << V[1] << v[1];
    actab << "(2)" << X[2] << V[2] << v[2];
    m_msgs << actab.getFormattedString();

    actab = ACTable(4);
    actab << " . | vx | vy | vz ";
    actab.addHeaderLines();
    actab << "Fvx" << DAMPING_MATRIX(0, 0) << DAMPING_MATRIX(0, 1) << DAMPING_MATRIX(0, 2);
    actab << "Fvy" << DAMPING_MATRIX(1, 0) << DAMPING_MATRIX(1, 1) << DAMPING_MATRIX(1, 2);
    actab << "Fvz" << DAMPING_MATRIX(2, 0) << DAMPING_MATRIX(2, 1) << DAMPING_MATRIX(2, 2);
    m_msgs << actab.getFormattedString();

    actab = ACTable(4);
    actab << " (local) | u1 | u2 | u3";
    actab << "Fx" << COEFF_MATRIX_INV(0, 0) << COEFF_MATRIX_INV(0, 1) << COEFF_MATRIX_INV(0, 2);
    actab << "Fy" << COEFF_MATRIX_INV(1, 0) << COEFF_MATRIX_INV(1, 1) << COEFF_MATRIX_INV(1, 2);
    actab << "Fz" << COEFF_MATRIX_INV(2, 0) << COEFF_MATRIX_INV(2, 1) << COEFF_MATRIX_INV(2, 2);
    m_msgs << actab.getFormattedString();
    return (true);
}




