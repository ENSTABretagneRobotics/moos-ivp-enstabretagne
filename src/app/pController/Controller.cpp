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

    if(key == "KP_HEADING") 
      KP_HEADING = msg.GetDouble();
    else if(key == "KD_HEADING") 
      KD_HEADING = msg.GetDouble();
    else if(key == "KI_HEADING") 
      KI_HEADING = msg.GetDouble();
    else if(key == "MIN_HEADING_ERROR") 
      MIN_HEADING_ERROR = msg.GetDouble();


    else if(key == "KP_DEPTH") 
      KP_DEPTH = msg.GetDouble();
    else if(key == "KD_DEPTH") 
      KD_DEPTH = msg.GetDouble();
    else if(key == "KI_DEPTH") 
      KI_DEPTH = msg.GetDouble();
    else if(key == "MIN_DEPTH_ERROR") 
      MIN_DEPTH_ERROR = msg.GetDouble();
    

    else if(key == "K_SPEED_FACTOR") 
      K_SPEED_FACTOR = msg.GetDouble();


    else if(key == "IMU_YAW") 
      //The IMU-YWA rate received from iXSensIMU is in interval [-180;180] 
      //and the controller is intended to work with [0;360] values
      actual_heading = msg.GetDouble() + 180; 

    else if(key == "KELLER_DEPTH") 
      actual_depth = msg.GetDouble();


    else if(key == "DESIRED_SPEED") 
      desired_speed = msg.GetDouble();
    else if(key == "DESIRED_DEPTH") 
      desired_depth = msg.GetDouble();
    else if(key == "DESIRED_HEADING") 
      desired_heading = msg.GetDouble();


    else if(key == "OPERATION_MODE") 
      OPERATION_MODE = msg.GetString();
    else if(key == "MAX_FORWARD_SPEED") 
      MAX_FORWARD_SPEED = msg.GetDouble();
    else if(key == "MAX_BACKWARD_SPEED") 
      MAX_BACKWARD_SPEED = msg.GetDouble();


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
  


  /*** Check for correct mode of operation of the robot ***/
  //The controller loop will only run if the robot is in
  //total or semi autonomus mode
  if(OPERATION_MODE == "AUTONOMUS" || OPERATION_MODE == "SEMI-AUTONOMUS")
  {
    //run the controller loop

    //get delta_t;
    delta_t = MOOSTime() - old_MOOSTime;
    old_MOOSTime = MOOSTime();


    /** HEADING CONTROLLER */

      //calculate the error (actual heading - desired heading)
      error_heading_rad = (actual_heading - desired_heading) * PI/180.0;

      //wrap the error into [-PI;PI]
      error_heading_rad = -2.0*atan(tan(error_heading_rad/2.0));

      //convert error to degress
      error_heading_degrees = error_heading_rad *180.0/PI;

      //check if the error is bigger than the min error parameter
      if(fabs(error_heading_degrees) > MIN_HEADING_ERROR)
      {
        //error is different from zero, calculate PID control


        //proportional value
        proportional_heading = KP_HEADING * error_heading_degrees;
        derivative_heading = (error_heading_degrees - old_error_heading)/delta_t;
        integral_heading =  integral_heading + ((error_heading_degrees + antiWindUp_heading)*delta_t); //anti-wind up below


        //calculate output command value
        FR_ROTATIONAL_FORCE =  proportional_heading
                          + KD_HEADING * derivative_heading
                          + KI_HEADING * integral_heading;


        //OUTPUT SATURATION CHECK
        //check if the oubput command value is between the max output value
        if(fabs(FR_ROTATIONAL_FORCE)> max_rotational_force)
         {

            antiWindUp_heading = FR_ROTATIONAL_FORCE;

          //output value is saturated
          if(FR_ROTATIONAL_FORCE > 0)
          {
            //positive saturation
            FR_ROTATIONAL_FORCE = max_rotational_force;

          }
          else
          {
            //negative saturation
            FR_ROTATIONAL_FORCE = -max_rotational_force;
          }

          //Anti Wind-Up for integral value of the PID
          //for more information visit: http://fr.mathworks.com/help/simulink/examples/anti-windup-control-using-a-pid-controller.html
          antiWindUp_heading =  (max_rotational_force - antiWindUp_heading);

         }//end of if check saturation 
         else
            antiWindUp_heading = 0;

      
      }//end if check error
      else
      {
        //the error is small enought to be considered as zero
        //reset the output command variable
        FR_ROTATIONAL_FORCE = 0;

        //reset the integral term
        integral_heading = 0;

      }//end of else

      //record the error heading for future iterations
      old_error_heading = error_heading_degrees;

   
    /** END OF HEADING CONTROLLER */




    /** SPEED CONTROLLER **/

      //maps a desired speed in meters/second in a force between [-MAX_FORWARD_SPEED;+MAX_FORWARD_SPEED]
      //this mapping depends on the MAX_FORWARD_SPEED and MAX_BACKWARD_SPEED of the vehicle
      if(desired_speed > 0)
        //forward speed
        FX_FORWARD_FORCE = (desired_speed * max_forward_force / MAX_FORWARD_SPEED) * K_SPEED_FACTOR;
      else
        //backward speed
        FX_FORWARD_FORCE = desired_speed * max_forward_force / MAX_BACKWARD_SPEED * K_SPEED_FACTOR;


      //check for saturarion
      FX_FORWARD_FORCE = (FX_FORWARD_FORCE < max_forward_force) ? FX_FORWARD_FORCE: max_forward_force;
      FX_FORWARD_FORCE = (FX_FORWARD_FORCE > -max_forward_force) ? FX_FORWARD_FORCE : -max_forward_force;

    /** END OF SPEED CONTROLLER **/




    }//end of if(OPERATION_MODE == "AUTONOMUS" || OPERATION_MODE == "SEMI-AUTONOMUS")
    else
    {
       //don't run the controllers at all
      //erase the global variables
      error_heading_degrees = 0;
      error_heading_rad = 0;
      FX_FORWARD_FORCE = 0;
      FZ_VERTICAL_FORCE = 0;
      antiWindUp_heading = 0;

      //reset integral value
      integral_heading = 0;
      proportional_heading = derivative_heading = 0;

    }

    
    /*** Check for correct mode of operation of the robot ***/
    //The depth controller loop will only run if the robot is in
    //total, semi autonomus or DEPTH_CONTROL_ONLY mode

    if(OPERATION_MODE == "AUTONOMUS" || OPERATION_MODE == "SEMI-AUTONOMUS" || OPERATION_MODE == "DEPTH_CONTROL_ONLY")
    {
    /** DEPTH CONTROLLER */

      //depth controller is also a PID controller
      //for a bang-bang controller, you can give a high Kp and 0 value for Ki and Kd

      //calculate the error (actual depth - desired depth)
      error_depth = actual_depth - desired_depth;


      //check if the error is bigger than the min error parameter
      if(fabs(error_depth) > MIN_DEPTH_ERROR)
      {
        //error is different from zero, calculate PID control


        //proportional value
        proportional_depth = KP_DEPTH * error_depth;
        derivative_depth =  (error_depth - old_error_depth)/delta_t;
        integral_depth =  integral_depth + ((error_depth+antiWindUp_depth)*delta_t);

        //TODO limit integral term!?

        //calculate output command value
        FZ_VERTICAL_FORCE =  proportional_depth
                  + KD_DEPTH * derivative_depth
                  + KI_DEPTH * integral_depth;


        //OUTPUT SATURATION CHECK
        //check if the oubput command value is between the max output value
        if(fabs(FZ_VERTICAL_FORCE)> max_z_force)
         {

          antiWindUp_depth = FZ_VERTICAL_FORCE;

          //output value is saturated
          if(FZ_VERTICAL_FORCE > 0)
            FZ_VERTICAL_FORCE = max_z_force;      //positive saturation
          else
            FZ_VERTICAL_FORCE = -1 * max_z_force; //negative saturation

          antiWindUp_depth = (max_z_force - antiWindUp_depth);

         }//end of if check saturation 
         else
          antiWindUp_depth = 0;

      
      }//end if check error
      else
      {
        //the error is small enought to be considered as zero
        //reset the output command variable
        FZ_VERTICAL_FORCE = 0;

        //reset the integral term
        integral_depth = 0;

      }//end of else

      //record the error heading for future iterations
      old_error_depth = error_depth;


    /** END OF DEPTH CONTROLLER */



    

  }//end of if operation mode
  else
  {
    //don't run depth controller at all
    //erase the global variables
    error_depth = 0;
    FZ_VERTICAL_FORCE = 0;
    antiWindUp_depth = 0;

    //reset integral value
    integral_depth = 0;
    proportional_depth = derivative_depth = 0;

  }//end of else



  //Notify the output variables
  Notify("FR_ROTATIONAL_FORCE", FR_ROTATIONAL_FORCE);
  Notify("FZ_VERTICAL_FORCE", FZ_VERTICAL_FORCE);
  Notify("FX_FORWARD_FORCE", FX_FORWARD_FORCE);



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
  KP_HEADING = 1;                //proportional constant value
  KD_HEADING = 0;                //derivative constant value
  KI_HEADING = 0;                //integral constant value

  //Depth
  KP_DEPTH = 1;                //proportional constant value
  KD_DEPTH = 0;                //derivative constant value
  KI_DEPTH = 0;                //integral constant value

  //Speed
  K_SPEED_FACTOR = 1;          //k factor for the speed mapping, tipically = 1;

  //Error thresholds
  //both minimun errors below are threshold values for the controller loop
  
  //minimum heading error, must be at least the precision of the heading sensor
  MIN_HEADING_ERROR = 3; //[0, 180] //in degress
  //minimum heading error, must be at least the precision of the pressure sensor
  MIN_DEPTH_ERROR = 0.1; //in meters

  //Max output values
  max_rotational_force = 1;
  max_z_force = 1;
  max_forward_force = 1;


  /** EXTERNAL INPUT VARIABLES **/

  //Possible values for Operation Mode
  //1) MANUAL - when the vehicle motion is fully operated by human
  //2) SEMI-AUTONOMUS - when the controller is used but the setpoints are setted by human operation
  //3) AUTONOMUS - When the setpoints are setted by a higher level controller
  OPERATION_MODE = "MANUAL";  //When initialized, the operation mode must start as manual

  desired_heading = 0;   //setpoint for the heading in degrees [0, 360)
  desired_depth = 0;     //setpoint for depth in meters
  desired_speed = 0;     //setpoint for the speed in meters/second
  actual_heading = 0;    //actual heading value in degrees [0, 360)
  actual_depth = 0;      //actual depth value in meters
  antiWindUp_heading = 0;
  antiWindUp_depth = 0;

  MAX_FORWARD_SPEED = 3;   //max forward speed of the vehicle in meters/second
  MAX_BACKWARD_SPEED = 2.82;  //max backward speed of the vechicle in meters/second


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


    if(param == "KP_HEADING") 
    {
      KP_HEADING = atof(value.c_str());
      handled = true;
    } 
    else if(param == "KD_HEADING") 
    {
      KD_HEADING = atof(value.c_str());
      handled = true;
    } 

    else if(param == "KI_HEADING") 
    {
      KI_HEADING = atof(value.c_str());
      handled = true;
    } 

    else if(param == "MIN_HEADING_ERROR") 
    {
      MIN_HEADING_ERROR = atof(value.c_str());
      handled = true;
    } 



    else if(param == "KP_DEPTH") 
    {
      KP_DEPTH = atof(value.c_str());
      handled = true;
    } 

    else if(param == "KD_DEPTH") 
    {
      KD_DEPTH = atof(value.c_str());
      handled = true;
    } 

    else if(param == "KI_DEPTH") 
    {
      KI_DEPTH = atof(value.c_str());
      handled = true;
    } 

    else if(param == "MIN_DEPTH_ERROR") 
    {
      MIN_DEPTH_ERROR = atof(value.c_str());
      handled = true;
    } 



    else if(param == "K_SPEED_FACTOR") 
    {
      K_SPEED_FACTOR = atof(value.c_str());
      handled = true;
    } 


    else if(param == "MAX_FORWARD_SPEED") 
    {
      MAX_FORWARD_SPEED = atof(value.c_str());
      handled = true;
    } 

    else if(param == "MAX_BACKWARD_SPEED") 
    {
      MAX_BACKWARD_SPEED = atof(value.c_str());
      handled = true;
    } 



    else if(param == "MAX_ROTATIONAL_FORCE") 
    {
      max_rotational_force = atof(value.c_str());
      handled = true;
    } 

    else if(param == "MAX_Z_FORCE") 
    {
      max_z_force = atof(value.c_str());
      handled = true;
    } 

    else if(param == "MAX_FORWARD_FORCE") 
    {
      max_forward_force = atof(value.c_str());
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
  Register("KP_HEADING", 0);
  Register("KD_HEADING", 0);
  Register("KI_HEADING", 0);
  Register("MIN_HEADING_ERROR", 0);

  Register("KP_DEPTH", 0);
  Register("KD_DEPTH", 0);
  Register("KI_DEPTH", 0);
  Register("MIN_DEPTH_ERROR", 0);

  Register("K_SPEED_FACTOR", 0);

  Register("IMU_YAW", 0);
  Register("KELLER_DEPTH", 0);

  Register("DESIRED_SPEED", 0);
  Register("DESIRED_DEPTH", 0);
  Register("DESIRED_HEADING", 0);

  Register("OPERATION_MODE", 0);
  Register("MAX_FORWARD_SPEED", 0);
  Register("MAX_BACKWARD_SPEED", 0);

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


  m_msgs << "============================================ \n";
  m_msgs << "pController:                                      \n";
  m_msgs << "============================================ \n";
  m_msgs << "\n";
  m_msgs << "OPERATION_MODE: " << OPERATION_MODE;
  m_msgs << "\n";
  m_msgs << "\n";

  m_msgs << "=========================== \n";
  m_msgs << "Parameters:         \n";
  m_msgs << "PID        Kp   Kd   ki \n";
  m_msgs << "        ----------------- \n";
  m_msgs << "Heading |  "<< KP_HEADING <<"   "<< KD_HEADING <<"    "<< KI_HEADING <<"  \n";
  m_msgs << "Depth   |  "<< KP_DEPTH <<"   "<< KD_DEPTH <<"    "<< KI_DEPTH <<"  \n";
  m_msgs << "\n";
  m_msgs << "K_SPEED_FACTOR: " << K_SPEED_FACTOR;
  m_msgs << "\n";
  m_msgs << "MIN_HEADING_ERROR (degrees): " << MIN_HEADING_ERROR;
  m_msgs << "\n";
  m_msgs << "MIN_DEPTH_ERROR   (meters) : " << MIN_DEPTH_ERROR;
  m_msgs << "\n";
  m_msgs << "\n";

  m_msgs << "=========================== \n";
  m_msgs << "Desired / Mesured / error values:         \n";
  m_msgs << "Heading:  "<< desired_heading <<" / "<< actual_heading <<" / "<< error_heading_degrees <<" \n";
  m_msgs << "Depth:  "<< desired_depth <<" / "<< actual_depth <<" / "<< error_depth <<" \n";
  m_msgs << "\n";
  m_msgs << "Desired Speed =  "<< desired_speed <<"\n";
  m_msgs << "\n";


  m_msgs << "=========================== \n";
  m_msgs << "  PID Values                 \n";
  m_msgs << "          Prop. / Derv. / Intg.:         \n";
  m_msgs << "Heading:  "<< proportional_heading <<" / "<< derivative_heading <<" / "<< integral_heading <<" \n";
  m_msgs << "Depth:    "<< proportional_depth <<" / "<< derivative_depth <<" / "<< integral_depth <<" \n";
  m_msgs << "\n";




  m_msgs << "=========================== \n";
  m_msgs << "Output values:         \n";
  m_msgs << "Fx =  "<< FX_FORWARD_FORCE <<" \n";
  m_msgs << "Fr =  "<< FR_ROTATIONAL_FORCE <<"\n";
  m_msgs << "Fz =  "<< FZ_VERTICAL_FORCE <<" \n";
  m_msgs << "\n";


  return true;
}
