/************************************************************/
/*    NAME: Jeremy NICOLA                                              */
/*    ORGN: Toutatis AUVs - ENSTA Bretagne                                             */
/*    FILE: EstimSpeed.h                                          */
/*    DATE: 2015                             */
/************************************************************/

#ifndef EstimSpeed_HEADER
#define EstimSpeed_HEADER

#include "Eigen/Dense"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class EstimSpeed : public AppCastingMOOSApp {
public:
    EstimSpeed();
    ~EstimSpeed() {};
    void quit() {};

protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

protected: // Standard AppCastingMOOSApp function to overload 
    bool buildReport();

protected:
    void registerVariables();

private: // Configuration variables
    // Translation
    Eigen::Matrix3d COEFF_MATRIX;
    
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
    Eigen::Matrix3d COEFF_MATRIX_TRANSLATION;
    
    Eigen::Matrix3d COEFF_MATRIX_TRANSLATION_INV;
    
    Eigen::Matrix3d DAMPING_MATRIX;
    
    double MASS;
    
private: // Inputs
    double imu_yaw;
    Eigen::Vector3d u; // (u1,u2,u3)
    Eigen::Matrix3d rot; // local to global
    double old_MOOSTime;
private: //State variables
    bool mission_started;
    Eigen::Vector3d X; // X,Y,Z, repere global
    Eigen::Vector3d V; // VX,VY,VZ repere global
    Eigen::Vector3d v; // vx,vy,vz, repere local
    
};

#endif 
