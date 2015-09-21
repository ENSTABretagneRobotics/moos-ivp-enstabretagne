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

    ~EstimSpeed() {
    };

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
    Eigen::Matrix3d COEFF_MATRIX_INV;
    
    Eigen::Matrix3d DAMPING_MATRIX;
    
    double MASS;
    
    // Names of the topics we subscribe to
    std::string YAW_REGISTRATION_NAME;
    std::string U1_SUBSCRIPTION_NAME;
    std::string U2_SUBSCRIPTION_NAME;
    std::string U3_SUBSCRIPTION_NAME;
    std::string RESET_SUBSCRIPTION_NAME;
    std::string GPS_E_SUBSCRIPTION_NAME;
    std::string GPS_N_SUBSCRIPTION_NAME;
    std::string KELLER_DEPTH_SUBSCRIPTION_NAME;
    
    // Names of the topics we publish to
    std::string POS_DEAD_RECKONING_PUBLICATION_NAME;
    std::string POS_DEAD_RECKONING_E_PUBLICATION_NAME;
    std::string POS_DEAD_RECKONING_N_PUBLICATION_NAME;
    std::string POS_DEAD_RECKONING_U_PUBLICATION_NAME;
    
    std::string SPEED_LOCAL_DEAD_RECKONING_PUBLICATION_NAME;
    std::string SPEED_GLOBAL_DEAD_RECKONING_PUBLICATION_NAME;
    
private: // Inputs
    double imu_yaw;
    Eigen::Vector3d u; // (u1,u2,u3)
    Eigen::Matrix3d rot; // local to global
    Eigen::Vector3d X_GPS_PRESSURE; // We store the GPS and Pressure sensors positions here
    double old_MOOSTime;
private: //State variables
    Eigen::Vector3d X; // X,Y,Z, repere global
    Eigen::Vector3d V; // VX,VY,VZ repere global
    Eigen::Vector3d v; // vx,vy,vz, repere local
};

#endif 
