/************************************************************/
/*    NAME: Jeremy NICOLA                                              */
/*    ORGN: Toutatis AUVs - ENSTA Bretagne                                             */
/*    FILE: EstimSpeed.cpp                                        */
/*    DATE: 2015                                                */
/************************************************************/

#include <iterator>
#include <math.h>
#include "MBUtils.h"
#include "ACTable.h"
#include "EstimSpeed.h"

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
    COEFF_MATRIX(0, 1) = 1;
    COEFF_MATRIX(0, 2) = 0;
    COEFF_MATRIX(1, 0) = 1;
    COEFF_MATRIX(1, 1) = -1;
    COEFF_MATRIX(1, 2) = 0;
    COEFF_MATRIX(2, 0) = 0;
    COEFF_MATRIX(2, 1) = 0;
    COEFF_MATRIX(2, 2) = 1;

    COEFF_MATRIX_TRANSLATION = Matrix3d::Zero();

    COEFF_MATRIX_TRANSLATION.block<3, 1>(0, 0) = COEFF_MATRIX.block<3, 1>(0, 0);
    COEFF_MATRIX_TRANSLATION.block<3, 1>(0, 2) = COEFF_MATRIX.block<3, 1>(0, 2);

    COEFF_MATRIX_TRANSLATION_INV = COEFF_MATRIX_TRANSLATION.inverse();

    DAMPING_MATRIX = Matrix3d::Identity()*0.4;

    MASS = 20;

    imu_yaw = 0;
    u = Vector3d::Zero();
    rot = Matrix3d::Identity();

    X = Vector3d::Zero();
    V = Vector3d::Zero();
    v = Vector3d::Zero();

    old_MOOSTime = MOOSTime();
    init = false;
}

bool EstimSpeed::OnNewMail(MOOSMSG_LIST &NewMail) {
    AppCastingMOOSApp::OnNewMail(NewMail);

    old_MOOSTime = MOOSTime();

    MOOSMSG_LIST::iterator p;
    for (p = NewMail.begin(); p != NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();

        // We receive the IMU yaw, in degrees
        if (key == "IMU_YAW") {
            this->imu_yaw = msg.GetDouble();
            double cTheta = cos(MOOSDeg2Rad(imu_yaw));
            double sTheta = sin(MOOSDeg2Rad(imu_yaw));
            rot.block<2, 2>(0, 0) = (Matrix2d() << cTheta, -sTheta, sTheta, cTheta).finished();
        }// We receive LEFT thruster value, in [-1,1]]
        else if (key == "U1_SIDE_THRUSTER_ONE") {
            this->u(0) = msg.GetDouble();
        }// We receive RIGHT thruster value, in [-1,1]]
        else if (key == "U2_SIDE_THRUSTER_TWO") {
            this->u(1) = msg.GetDouble();
        }// We receive VERTICAL thruster value, in [-1,1]]
        else if (key == "U3_VERTICAL_THRUSTER") {
            this->u(2) = msg.GetDouble();
        } else if (key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }
    init = true;
    return (true);
}

bool EstimSpeed::OnConnectToServer() {
    registerVariables();
    return (true);
}

bool EstimSpeed::Iterate() {
    AppCastingMOOSApp::Iterate();
    if (init) {
        double delta_t = MOOSTime() - old_MOOSTime;
        old_MOOSTime = MOOSTime();

        X += delta_t*V;

        v += delta_t * (-DAMPING_MATRIX * (Vector3d(v(0) * v(0), v(1) * v(1), v(2) * v(2))) + COEFF_MATRIX_TRANSLATION_INV * u) / MASS;
        V = rot*v;

        stringstream ss;
        ss << X(0) << "," << X(1) << "," << X(2);

        Notify("POS_DEAD_RECKONING", ss.str());

        ss.clear();
        ss.str(std::string());

        ss << v(0) << "," << v(1) << "," << v(2);
        Notify("SPEED_LOCAL_DEAD_RECKONING", ss.str());

        ss.clear();
        ss.str(std::string());

        ss << V(0) << "," << V(1) << "," << V(2);
        ss.str(std::string());

        Notify("SPEED_GLOBAL_DEAD_RECKONING", ss.str());
    }

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
                COEFF_MATRIX_TRANSLATION.block<3, 1>(0, 0) = COEFF_MATRIX.block<3, 1>(0, 0);
                COEFF_MATRIX_TRANSLATION.block<3, 1>(0, 2) = COEFF_MATRIX.block<3, 1>(0, 2);

                COEFF_MATRIX_TRANSLATION_INV = COEFF_MATRIX_TRANSLATION.inverse();
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
        }
        // Initial speed
        else if(param=="V0")
        {
            vector<string> str_vector=parseString(value,',');
            
            if (str_vector.size()==3)
            {
                V << atof(str_vector[0].c_str()),atof(str_vector[1].c_str()),atof(str_vector[2].c_str());
            }else{
                reportUnhandledConfigWarning("Error while parsing V0, the initial speed: incorrect number of elements");
            }
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
    Register("IMU_YAW",0);
    Register("U1_SIDE_THRUSTER_ONE",0);
    Register("U2_SIDE_THRUSTER_TWO",0);
    Register("U3_VERTICAL_THRUSTER",0);
}

bool EstimSpeed::buildReport() {
    m_msgs << "============================================ \n";
    m_msgs << "pEstimSpeed                                  \n";
    m_msgs << "============================================ \n";
    m_msgs << "\n";

    ACTable actab(4);
    actab << "Yaw | Mass ";
    actab.addHeaderLines();
    actab << imu_yaw << MASS;
    m_msgs << actab.getFormattedString();

    actab = ACTable(4);
    actab << " . | X | V | v";
    actab.addHeaderLines();
    actab << "(0)" << X(0) << V(0) << v(0);
    actab << "(1)" << X(1) << V(1) << v(1);
    actab << "(2)" << X(2) << V(2) << v(2);
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
    actab << "Fx" << COEFF_MATRIX_TRANSLATION_INV(0, 0) << COEFF_MATRIX_TRANSLATION_INV(0, 1) << COEFF_MATRIX_TRANSLATION_INV(0, 2);
    actab << "Fy" << COEFF_MATRIX_TRANSLATION_INV(1, 0) << COEFF_MATRIX_TRANSLATION_INV(1, 1) << COEFF_MATRIX_TRANSLATION_INV(1, 2);
    actab << "Fz" << COEFF_MATRIX_TRANSLATION_INV(2, 0) << COEFF_MATRIX_TRANSLATION_INV(2, 1) << COEFF_MATRIX_TRANSLATION_INV(2, 2);
    m_msgs << actab.getFormattedString();
    return (true);
}




