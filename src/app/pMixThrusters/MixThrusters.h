/************************************************************/
/*    FILE: MixThrusters.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef MixThrusters_HEADER
#define MixThrusters_HEADER

#include "Eigen/Dense"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class MixThrusters : public AppCastingMOOSApp
{
  public:
    MixThrusters();
    ~MixThrusters() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // MixThrusters functions
    void saturationSigmoid(Eigen::Vector3d &u);
    void saturationNormalization(Eigen::Vector3d &u);
    double sensCorrection(double val);
    double fsign(double val);

  private: // Configuration variables
      Eigen::Matrix3d COEFF_MATRIX;
      std::string FX_SUBSCRIPTION_NAME;
      std::string RZ_SUBSCRIPTION_NAME;
      std::string FZ_SUBSCRIPTION_NAME;

      std::string U1_PUBLICATION_NAME;
      std::string U2_PUBLICATION_NAME;
      std::string U3_PUBLICATION_NAME;
      
      enum saturation_mods{NORMALIZATION,SIGMOID};
      int saturation_mod;

      double m_forward_coeff, m_backward_coeff;
      double m_sigmoid_coeff;
  private: // State variables
      Eigen::Vector3d desiredForces;
      
      Eigen::Vector3d u;
};

#endif 
