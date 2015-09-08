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
    double coeff_matrix[3][3];
    //std::array<std::array<double,3>,3> coeff_matrix;

    /** OUTPUT VARIABLES **/
    double side_thruster_one;
    double side_thruster_two;
    double vertical_thruster;

    /** EXTERNAL INPUT VARIABLES **/
    double rotational_force;     //[-1;1]
    double z_force;              //[-1;1]
    double forward_force;        //[-1;1]

    /** internal variables **/
    double max_thruster_value;
    double saturation_value;
    double saturated_thruster_value;

    std::string Operation_Mode;

  private: // State variables
    Pololu *m_pololu;
};

#endif 
