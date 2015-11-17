/************************************************************/
/*    FILE: Controller.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: MDS
/*    DATE: 2015
/************************************************************/

#ifndef Controller_HEADER
#define Controller_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class Controller : public AppCastingMOOSApp
{
  public:
    Controller();
    ~Controller() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // Controller functions


  private: // Configuration variables

    /** PARAMETERS **/

    //Controller constants
    //Heading
    double KP_HEADING;                //proportional constant value
    double KD_HEADING;                //derivative constant value
    double KI_HEADING;                //integral constant value

    //Depth
    double KP_DEPTH;                //proportional constant value
    double KD_DEPTH;                //derivative constant value
    double KI_DEPTH;                //integral constant value

    //Speed
    double K_SPEED_FACTOR;          //k factor for the speed mapping, tipically = 1;

    //Error thresholds
    //both minimun errors below are threshold values for the controller loop
    
    //minimum heading error, must be at least the precision of the heading sensor
    double MIN_HEADING_ERROR; //[0, 180]
    //minimum heading error, must be at least the precision of the pressure sensor
    double MIN_DEPTH_ERROR;

    //Max output values
    double max_rotational_force;
    double max_z_force;
    double max_forward_force;


    /** EXTERNAL INPUT VARIABLES **/
    std::string OPERATION_MODE;  //TODO set the operation mode globally in the Mission

    double desired_heading;   //setpoint for the heading in degrees [0, 360)
    double desired_depth;     //setpoint for depth in meters
    double desired_speed;     //setpoint for the speed in meters/second
    double actual_heading;    //actual heading value in degrees [0, 360)
    double actual_depth;      //actual depth value in meters

    double MAX_FORWARD_SPEED;   //max forward speed of the vehicle in meters/second
    double MAX_BACKWARD_SPEED;  //max backward speed of the vechicle in meters/second


    /** GLOBAL VARIABLES **/
    float old_MOOSTime;
    float old_error_heading;
    float old_error_depth;

    double error_heading_degrees;
    double error_heading_rad;
    double error_depth;
    double delta_t;
    
    /** PID VARIABLES **/
    double proportional_heading;
    double derivative_heading;
    double integral_heading;
    double proportional_depth;
    double derivative_depth;
    double integral_depth;
    double antiWindUp_heading;
    double antiWindUp_depth;

    /** OUTPUT VARIABLES **/
    double FR_ROTATIONAL_FORCE;  //positive value means clockwise direction
    double FZ_VERTICAL_FORCE;    //positive value means surface direction
    double FX_FORWARD_FORCE;     //positive valua means forward direction



  private: // State variables
    

};

#endif 
