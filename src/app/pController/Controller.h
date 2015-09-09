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
    double kp_heading;                //proportional constant value
    double kd_heading;                //derivative constant value
    double ki_heading;                //integral constant value

    //Depth
    double kp_depth;                //proportional constant value
    double kd_depth;                //derivative constant value
    double ki_depth;                //integral constant value

    //Speed
    double k_speed_factor;          //k factor for the speed mapping, tipically = 1;

    //Error thresholds
    //both minimun errors below are threshold values for the controller loop
    
    //minimum heading error, must be at least the precision of the heading sensor
    double min_heading_error; //[0, 180]
    //minimum heading error, must be at least the precision of the pressure sensor
    double min_depth_error;

    //Max output values
    double max_rotational_force;
    double max_z_force;
    double max_forward_force;


    /** EXTERNAL INPUT VARIABLES **/
    std::string Operation_Mode;  //TODO set the operation mode globally in the Mission

    double desired_heading;   //setpoint for the heading in degrees [0, 360)
    double desired_depth;     //setpoint for depth in meters
    double desired_speed;     //setpoint for the speed in meters/second
    double actual_heading;    //actual heading value in degrees [0, 360)
    double actual_depth;      //actual depth value in meters

    double max_forward_speed;   //max forward speed of the vehicle in meters/second
    double max_backward_speed;  //max backward speed of the vechicle in meters/second


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
    double rotational_force;  //positive value means clockwise direction
    double z_force;           //positive value means surface direction
    double forward_force;     //positive valua means forward direction



  private: // State variables
    

};

#endif 
