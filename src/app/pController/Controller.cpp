/************************************************************/
/*    FILE: Controller.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: MDS
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Controller.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Controller::Controller()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Controller::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

    #if 0 // Keep these around just for template
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString(); 
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif

    if(key == "kp_heading") 
      kp_heading = msg.GetDouble();
    else if(key == "kd_heading") 
      kd_heading = msg.GetDouble();
    else if(key == "ki_heading") 
      ki_heading = msg.GetDouble();
    else if(key == "min_heading_error") 
      min_heading_error = msg.GetDouble();


    else if(key == "kp_depth") 
      kp_depth = msg.GetDouble();
    else if(key == "kd_depth") 
      kd_depth = msg.GetDouble();
    else if(key == "ki_depth") 
      ki_depth = msg.GetDouble();
    else if(key == "min_depth_error") 
      min_depth_error = msg.GetDouble();
    

    else if(key == "k_speed_factor") 
      k_speed_factor = msg.GetDouble();


    else if(key == "IMU_YAW") 
      actual_heading = msg.GetDouble();
    else if(key == "KELLER_DEPTH") 
      actual_depth = msg.GetDouble();


    else if(key == "Desired_Speed") 
      desired_speed = msg.GetDouble();
    else if(key == "Desired_Depth") 
      desired_depth = msg.GetDouble();
    else if(key == "Desired_Heading") 
      desired_heading = msg.GetDouble();


    else if(key == "Operation_Mode") 
      Operation_Mode = msg.GetString();
    else if(key == "max_forward_speed") 
      max_forward_speed = msg.GetDouble();
    else if(key == "max_backward_speed") 
      max_backward_speed = msg.GetDouble();


    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Controller::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Controller::Iterate()
{
  AppCastingMOOSApp::Iterate();

  
  /** OUTPUT VARIABLES **/
  double rotational_force;  //positive value means clockwise direction
  double z_force;           //positive value means surface direction
  double forward_force;     //positive valua means forward direction


  /** INTERNAL VARIABLES  (local) **/
  double error_heading;
  double proportional_heading;
  double derivative_heading;
  double integral_heading;
  double error_depth;
  double proportional_depth;
  double derivative_depth;
  double integral_depth;
  double delta_t;


  /*** Check for correct mode of operation of the robot ***/
  //The controller loop will only run if the robot is in
  //total or semi autonomus mode
  if(Operation_Mode == "Autonomus" || Operation_Mode == "Semi-Autonomus")
  {
    //run the controller loop

    //get delta_t;
    delta_t = MOOSTime() - old_MOOSTime;
    old_MOOSTime = MOOSTime();


    /** HEADING CONTROLLER */

      //calculate the error (actual heading - desired heading)
      error_heading = actual_heading - desired_heading;

      //wrap the error to +-180 degress
      //assuming that the error is smallest or equal to 360
      if(error_heading > 180)
      {
        error_heading = error_heading - 360;

      }//end of if

      //check if the error is bigger than the min error parameter
      if(fabs(error_heading) > min_heading_error)
      {
        //error is different from zero, calculate PID control


        //proportional value
        proportional_heading = kp_heading * error_heading;
        derivative_heading = (error_heading - old_error_heading)/delta_t;
        integral_heading =  integral_heading + (error_heading*delta_t);

        //TODO limit integral term!?

        //calculate output command value
        rotational_force =  proportional_heading
                          + kd_heading * derivative_heading
                          + ki_heading * integral_heading;


        //OUTPUT SATURATION CHECK
        //check if the oubput command value is between the max output value
        if(fabs(rotational_force)> max_rotational_force)
         {
          //output value is saturated
          if(rotational_force > 0)
          {
            //positive saturation
            rotational_force = max_rotational_force;
          }
          else
          {
            //negative saturation
            rotational_force = -max_rotational_force;
          }

         }//end of if check saturation 

      
      }//end if check error
      else
      {
        //the error is small enought to be considered as zero
        //reset the output command variable
        rotational_force = 0;

        //reset the integral term
        integral_heading = 0;

      }//end of else

      //record the error heading for future iterations
      old_error_heading = error_heading;

   
    /** END OF HEADING CONTROLLER */


    /** DEPTH CONTROLLER */

      //depth controller is also a PID controller
      //for a bang-bang controller, you can give a high Kp and 0 value for Ki and Kd

      //calculate the error (actual depth - desired depth)
      error_depth = actual_depth - desired_depth;


      //check if the error is bigger than the min error parameter
      if(fabs(error_depth) > min_depth_error)
      {
        //error is different from zero, calculate PID control


        //proportional value
        proportional_depth = kp_depth * error_depth;
        derivative_depth =  (error_depth - old_error_depth)/delta_t;
        integral_depth =  integral_depth + (error_depth*delta_t);

        //TODO limit integral term!?

        //calculate output command value
        z_force =  proportional_depth
                  + kd_depth * derivative_depth
                  + ki_depth * integral_depth;


        //OUTPUT SATURATION CHECK
        //check if the oubput command value is between the max output value
        if(fabs(z_force)> max_z_force)
         {

          //output value is saturated
          if(z_force > 0)
            z_force = max_z_force;      //positive saturation
          else
            z_force = -1 * max_z_force; //negative saturation

         }//end of if check saturation 

      
      }//end if check error
      else
      {
        //the error is small enought to be considered as zero
        //reset the output command variable
        z_force = 0;

        //reset the integral term
        integral_depth = 0;

      }//end of else

      //record the error heading for future iterations
      old_error_depth = error_depth;


    /** END OF DEPTH CONTROLLER */



    /** SPEED CONTROLLER **/

      //maps a desired speed in meters/second in a force between [-max_forward_speed;+max_forward_speed]
      //this mapping depends on the max_forward_speed and max_backward_speed of the vehicle
      if(desired_speed > 0)
        //forward speed
        forward_force = (desired_speed * max_forward_force / max_forward_speed) * k_speed_factor;
      else
        //backward speed
        forward_force = desired_speed * max_forward_force / max_backward_speed * k_speed_factor;


      //check for saturarion
      if(forward_force>max_forward_force)
        forward_force = max_forward_force;    //positive saturation
      else if(forward_force<max_forward_force)
        forward_force = - max_forward_force;  //negative saturation

    /** END OF SPEED CONTROLLER **/

  }//end of if operation mode
  else
  {
    //don't run anything at all
    //erase the global variables
    //TODO: Erase the used variables
    error_depth = 0;
    error_heading = 0;
    rotational_force = 0;
    z_force = 0;

    //reset integral value
    integral_heading = 0;
    integral_depth = 0;

  }//end of else



  //Notify the output variables
  Notify("rotational_force", rotational_force);
  Notify("z_force", z_force);
  Notify("forward_force", forward_force);



  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Controller::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();



  //default config
  //Heading
  kp_heading = 1;                //proportional constant value
  kd_heading = 0;                //derivative constant value
  ki_heading = 0;                //integral constant value

  //Depth
  kp_depth = 1;                //proportional constant value
  kd_depth = 0;                //derivative constant value
  ki_depth = 0;                //integral constant value

  //Speed
  k_speed_factor = 1;          //k factor for the speed mapping, tipically = 1;

  //Error thresholds
  //both minimun errors below are threshold values for the controller loop
  
  //minimum heading error, must be at least the precision of the heading sensor
  min_heading_error = 3; //[0, 180] //in degress
  //minimum heading error, must be at least the precision of the pressure sensor
  min_depth_error = 0.1; //in meters

  //Max output values
  max_rotational_force = 1;
  max_z_force = 1;
  max_forward_force = 1;


  /** EXTERNAL INPUT VARIABLES **/

  //Possible values for Operation Mode
  //1) Manual - when the vehicle motion is fully operated by human
  //2) Semi-Autonomus - when the controller is used but the setpoints are setted by human operation
  //3) Autonomus - When the setpoints are setted by a higher level controller
  Operation_Mode = "Manual";  //When initialized, the operation mode must be manual

  desired_heading = 0;   //setpoint for the heading in degrees [0, 360)
  desired_depth = 0;     //setpoint for depth in meters
  desired_speed = 0;     //setpoint for the speed in meters/second
  actual_heading = 0;    //actual heading value in degrees [0, 360)
  actual_depth = 0;      //actual depth value in meters

  max_forward_speed = 3;   //max forward speed of the vehicle in meters/second
  max_backward_speed = 2.82;  //max backward speed of the vechicle in meters/second


  old_MOOSTime = 0.0;
  old_error_heading = 0.0;
  old_error_depth = 0.0;


  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  sParams.reverse();
  for(p = sParams.begin() ; p != sParams.end() ; p++)
  {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;
    bool handled = false;


    if(param == "kp_heading") 
    {
      kp_heading = atof(value.c_str());
      handled = true;
    } 
    else if(param == "kd_heading") 
    {
      kd_heading = atof(value.c_str());
      handled = true;
    } 

    else if(param == "ki_heading") 
    {
      ki_heading = atof(value.c_str());
      handled = true;
    } 

    else if(param == "min_heading_error") 
    {
      min_heading_error = atof(value.c_str());
      handled = true;
    } 



    else if(param == "kp_depth") 
    {
      kp_depth = atof(value.c_str());
      handled = true;
    } 

    else if(param == "kd_depth") 
    {
      kd_depth = atof(value.c_str());
      handled = true;
    } 

    else if(param == "ki_depth") 
    {
      ki_depth = atof(value.c_str());
      handled = true;
    } 

    else if(param == "min_depth_error") 
    {
      min_depth_error = atof(value.c_str());
      handled = true;
    } 



    else if(param == "k_speed_factor") 
    {
      k_speed_factor = atof(value.c_str());
      handled = true;
    } 


    else if(param == "max_forward_speed") 
    {
      max_forward_speed = atof(value.c_str());
      handled = true;
    } 

    else if(param == "max_backward_speed") 
    {
      max_backward_speed = atof(value.c_str());
      handled = true;
    } 
      



    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  registerVariables();  
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void Controller::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("kp_heading", 0);
  Register("kd_heading", 0);
  Register("ki_heading", 0);
  Register("min_heading_error", 0);

  Register("kp_depth", 0);
  Register("kd_depth", 0);
  Register("ki_depth", 0);
  Register("min_depth_error", 0);

  Register("k_speed_factor", 0);

  Register("IMU_YAW", 0);
  Register("KELLER_DEPTH", 0);

  Register("Desired_Speed", 0);
  Register("Desired_Depth", 0);
  Register("Desired_Heading", 0);

  Register("Operation_Mode", 0);
  Register("max_forward_speed", 0);
  Register("max_backward_speed", 0);

}

//------------------------------------------------------------
// Procedure: buildReport()

bool Controller::buildReport() 
{
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
