/************************************************************/
/*    FILE: Saucisse.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef Saucisse_HEADER
#define Saucisse_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "Pololu.h"



class Saucisse : public AppCastingMOOSApp
{
  public:
    Saucisse();
    ~Saucisse();

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // Saucisse functions
    void CalcThrustersValues();

  private: // Configuration variables
    std::string m_device_name;

    /** COEFFICIENT MATRIX **/
    double COEFF_MATRIX[3][3];
    //std::array<std::array<double,3>,3> COEFF_MATRIX;

    /** OUTPUT VARIABLES **/
    double U1_SIDE_THRUSTER_ONE;
    double U2_SIDE_THRUSTER_TWO;
    double U3_VERTICAL_THRUSTER;

    /** EXTERNAL INPUT VARIABLES **/
    double FR_ROTATIONAL_FORCE;     //[-1;1]
    double FZ_VERTICAL_FORCE;              //[-1;1]
    double FX_FORWARD_FORCE;        //[-1;1]

    /** internal variables **/
    double max_thruster_value;
    double saturation_value;
    double saturated_thruster_value;

    std::string OPERATION_MODE;

  private: // State variables
    Pololu *m_pololu;
};

#endif 
