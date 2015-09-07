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

    if(key == "FOO") 
      cout << "great!";

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

  /** PARAMETERS **/

  //both minimun errors below are threshold values for the controller loop

  //minimum heading error, must be at least the precision of the heading sensor
  double min_heading_error; //[0, 180]
  //minimum heading error, must be at least the precision of the pressure sensor
  double min_depth_error;


  //Controller constants
  double kp;                //proportional constant value
  double kd;                //derivative constant value
  double ki;                //integral constant value


  //Max output values
  double max_rotational_force = 1.0;
  double max_z_force = 1.0;


  /** EXTERNAL INPUT VARIABLES **/
  int Operation_Mode;  //TODO set the operation mode globally in the Mission

  double desired_Heading;   //setpoint for the heading in degrees [0, 360)
  double desired_Depth;     //setpoint for depth in meters
  double actual_heading;    //actual heading value in degrees [0, 360)
  double actual_depth;      //actual depth value in meters


  /** OUTPUT VARIABLES **/
  double rotational_force;  //positive value means clockwise direction
  double z_force;           //positive value means surface direction



  /** INTERNAL VARIABLES  (local) **/
  double error_heading;
  double proportional_heading:
  double derivative_heading;
  double integral_heading;
  double error_depth;
  double proportional_depth:
  double derivative_depth;
  double integral_depth;
  double delta_t;


  /** GLOBAL VARIABLES **/
  //TODO: remove these variables from here 
  //and put them into a global context
  float old_MOOSTime;
  float old_error_heading;
  float old_error_depth;


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
      error_heading = actual_heading - desired_Heading;

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
        proportional_heading = kp * error;
        derivative_heading = (error_heading - old_error_heading)/delta_t;
        integral_heading =  (integral_heading + (error_heading*delta_t);

        //TODO limit integral term!?

        //calculate output command value
        rotational_force =  proportional_heading
                          + kd * derivative_heading
                          + ki * integral_heading;


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
            rotational_force = -1 * max_rotational_force;
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

      //calculate the error (actual depth - desired depth)
      error_depth = actual_depth - desired_Depth;


      //check if the error is bigger than the min error parameter
      if(fabs(error_depth) > min_depth_error)
      {
        //error is different from zero, calculate PID control


        //proportional value
        proportional_depth = kp * error;
        derivative_depth = kd * (error_depth - old_error_depth)/delta_t;
        integral_depth =  (integral_depth + (error_depth*delta_t);

        //TODO limit integral term!?

        //calculate output command value
        rotational_force =  proportional_depth
                          + derivative_depth
                          + ki * integral_depth;


        //OUTPUT SATURATION CHECK
        //check if the oubput command value is between the max output value
        if(fabs(z_force)> max_z_force)
         {
          //output value is saturated
          if(z_force > 0)
          {
            //positive saturation
            z_force = max_z_force;
          }
          else
          {
            //negative saturation
            z_force = -1 * max_z_force;
          }

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


  }//end of if operation mode
  else
  {
    //don't run anything at all
    //erase the global variables
    //TODO: Erase the used variables
    error = 0;
    rotational_force = 0;
    z_force = 0;

    //reset integral value
    integral_heading = 0;
    integral_depth = 0;

  }//end of else



  //Notify the output variables
  Notify("rotational_force", rotational_force);
  Notify("z_force", z_force);



  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Controller::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

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

    if(param == "FOO")
    {
      handled = true;
    }

    else if(param == "BAR")
    {
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
