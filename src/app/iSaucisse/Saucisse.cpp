/************************************************************/
/*    FILE: Saucisse.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Saucisse.h"


using namespace std;

//---------------------------------------------------------
// Constructor

Saucisse::Saucisse()
{

}

//---------------------------------------------------------
// Denstructor

Saucisse::~Saucisse()
{
  delete m_pololu;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Saucisse::OnNewMail(MOOSMSG_LIST &NewMail)
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

    /*if(key == "POWER_CAMERA1")
    {
      int success = m_pololu->turnOnBistableRelay(2, 3, (int)msg.GetDouble() == 1);
      Notify("POWERED_CAMERA1", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_CAMERA2")
    {
      int success = m_pololu->turnOnBistableRelay(4, 5, (int)msg.GetDouble() == 1);
      Notify("POWERED_CAMERA2", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else */if(key == "POWER_MODEM")
    {
      int success = m_pololu->turnOnBistableRelay(6, 7, (int)msg.GetDouble() == 1);
      Notify("POWERED_MODEM", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_MODEM_EA")
    {
      int success = m_pololu->turnOnBistableRelay(8, 9, (int)msg.GetDouble() == 1);
      Notify("POWERED_MODEM_EA", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_SONAR")
    {
      int success = m_pololu->turnOnBistableRelay(10, 11, (int)msg.GetDouble() == 1);
      Notify("POWERED_SONAR", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_ECHOSOUNDER")
    {
      int success = m_pololu->turnOnBistableRelay(12, 13, (int)msg.GetDouble() == 1);
      Notify("POWERED_ECHOSOUNDER", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    /** CONTROLLER FORCE VALUES UPDATE**/
    else if(
      (Operation_Mode == "Autonomus" || Operation_Mode == "Semi-Autonomus") && 
      (key == "rotational_force" || key == "z_force" || key == "forward_force"))
    {

      //update the force value
      if(key == "rotational_force")
        rotational_force =  msg.GetDouble();
      else if(key == "forward_force")
        forward_force =  msg.GetDouble();
      else if(key == "z_force")
        z_force =  msg.GetDouble();

      //execute the calculation of the Thurster Values;
      Saucisse::CalcThrustersValues();
    }

    /** JOYSTICK FORCE VALUES UPDATE**/
    else if(
      (Operation_Mode == "Manual") && 
      (key == "DESIRED_ELEVATOR" || key == "DESIRED_THRUST" || key == "DESIRED_RUDDER"))
    {

      //update the force value
      if(key == "DESIRED_RUDDER")
        rotational_force =  msg.GetDouble();
      else if(key == "DESIRED_THRUST")
        forward_force =  msg.GetDouble();
      else if(key == "DESIRED_ELEVATOR")
        z_force =  msg.GetDouble();

      //execute the calculation of the Thurster Values;
      Saucisse::CalcThrustersValues();
    }

    else if(key == "Operation_Mode") 
      Operation_Mode = msg.GetString();

    /*else if(key == "POWER_GPS")
    {
      int success = m_pololu->turnOnBistableRelay(14, 15, (int)msg.GetDouble() == 1);
      Notify("POWERED_GPS", success >= 0 ? (int)msg.GetDouble() : -1);
    }*/

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }


  //Calculate the thrusters values
  Saucisse::CalcThrustersValues();

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Saucisse::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Saucisse::Iterate()
{
  AppCastingMOOSApp::Iterate();

  string error_message;
  bool pololu_ok = m_pololu->isReady(error_message);
  Notify("SAUCISSE_POLOLU_STATUS", pololu_ok ? "ok" : error_message);

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Saucisse::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  /** DEFAULT VALUES **/

  /*
    default values for coeff_matrix

       Fx   Fr   Fz
      ---------------
  u1 |  1   1   0 
  u2 |  1   1   0
  u3 |  0   0   1   

  coeff_matrix = { 
    {  1 ,  1 , 0  }, 
    {  1 ,  1 , 0  }, 
    {  0,   0 , 1  }
  };
  */
  coeff_matrix[0][0] = 1;
  coeff_matrix[0][1] = 1;
  coeff_matrix[0][2] = 0;
  coeff_matrix[1][0] = -1;
  coeff_matrix[1][1] = -1;
  coeff_matrix[1][2] = 0;
  coeff_matrix[2][0] = 0;
  coeff_matrix[2][1] = 0;
  coeff_matrix[2][2] = 1;

  max_thruster_value = 1; //[-1;1]

  //zero value means thursters stopped
  side_thruster_one = 0; 
  side_thruster_two = 0;
  vertical_thruster = 0;

  /** variable initialization **/
  rotational_force = 0;     
  z_force = 0;
  forward_force = 0;
  saturation_value = 0;
  saturated_thruster_value = 0;
  Operation_Mode = "Manual";

  /** END OF DEFAULT VALUES **/


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

    if(param == "DEVICE_NAME")
    {
      m_device_name = value;
      handled = true;
    }
    else if(param == "max_thruster_value")
    {
      max_thruster_value = atof(value.c_str());
      handled = true;
    }
    else if(param == "coeff_matrix")
    {
      vector<string> str_vector = parseString(value.c_str(), ',');

      //check for correct array size of 9 elements (3x3)
      if(str_vector.size() == 9)
      {
          //size is correct, put the values into the coeff_matrix
          for(unsigned int i=0; i<str_vector.size(); i++)
          {
            for(unsigned int j=0; i<3; j++)
            {
              //get the string and convert it into value
              coeff_matrix[i][j] = atof(str_vector[i+j].c_str());
            }
          }
      }
      else
      {
        //incorrect array size
        //send warning
        reportUnhandledConfigWarning("Error while parsing coeff_matrix: incorrect number of elements");

      }//end of else

      handled = true;
    }
    



    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  m_pololu = new Pololu(m_device_name);
  registerVariables();  
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void Saucisse::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("POWER_*", "*", 0);

  Register("rotational_force", 0);
  Register("z_force", 0);
  Register("forward_force", 0);
  Register("Operation_Mode", 0);

  Register("DESIRED_RUDDER", 0);
  Register("DESIRED_THRUST", 0);
  Register("DESIRED_ELEVATOR", 0);

}

//------------------------------------------------------------
// Procedure: buildReport()

bool Saucisse::buildReport() 
{
  m_msgs << "============================================ \n";
  m_msgs << "iSaucisse:                                      \n";
  m_msgs << "============================================ \n";
  m_msgs << "\n";
  
  string error_message;
  bool pololu_ok = m_pololu->isReady(error_message);
  m_msgs << "Status: " << (pololu_ok ? "ok" : error_message);

  return true;
}


void Saucisse::CalcThrustersValues()
{

  /** COEFFICIENT MATRIX 

    double coeff_matrix = [3][3]

       Fx   Fr   Fz
      ---------------
  u1 | a1   b1   c1 
  u2 | a2   b2   c2
  u3 | a3   b3   c3
  
  u1 and u2 are the side thrusters
  u3 is the vertical thrusters
  Fx is the force in the forward-reverse direction
  Fr is the roll (rotational) force
  Fz is the vertical force


  //c1, c2, a3 and b3 are tipically equal to ZERO
  //c3 is tipically equal to one

  */



  /** CALCULATION OF THE SIDE THRUSTERS VALUES **/
  
  //u1 = a1 * Fx + b1 * Fr + c1 * Fz
  side_thruster_one = coeff_matrix[0][0] * forward_force + coeff_matrix[0][1] * rotational_force + coeff_matrix[0][2] * z_force;

  //u2 = a2 * Fx + b2 * Fr + c2 * Fz
  side_thruster_two = coeff_matrix[1][0] * forward_force + coeff_matrix[1][1] * rotational_force + coeff_matrix[1][2] * z_force;





  /** CHECK FOR SATURATION of the side thrusters **/
    if(fabs(side_thruster_one) > max_thruster_value || fabs(side_thruster_two) > max_thruster_value)
    {
      //one of the two thrusters are saturated, remove the saturation
      //find out which one of the thursters is saturated
      if(fabs(side_thruster_one) > fabs(side_thruster_two))
      {
        //thruster one is saturated
        saturated_thruster_value = side_thruster_one;
        saturation_value = fabs(side_thruster_one) - max_thruster_value;
      
      }//end of if
      else
      {
        //thruster two is saturated
        saturated_thruster_value = side_thruster_two;
        

      }//end of else

      //calculate the saturation value, which will be removed from the thursters values
      saturation_value = fabs(side_thruster_two) - max_thruster_value;

      //check if the saturation is in (a)forward or (b)reverse direction
      if(saturated_thruster_value > 0)
      {
        //(a) saturation is in forward direction (positive) so we must REDUCE the saturated value
        side_thruster_one = side_thruster_one - saturation_value;
        side_thruster_two = side_thruster_two - saturation_value;
        //from now we have non saturated value for both thrusters
      }
      else
      {
        //(b) saturation is in reverse direction (negative) so we must SUM the saturated value in order to "desaturate" the thrusters output values
        side_thruster_one = side_thruster_one + saturation_value;
        side_thruster_two = side_thruster_two + saturation_value;
        //from now we have non saturated value for both thrusters
      }

    }//end of if saturation check
    //else - the thrusters values are not saturated so they can be sent to the thursters whitout additional manipulation

  /** END OF CHECK FOR SATURATION of the side thrusters **/




  /** CALCULATION OF THE VERTICAL THRUSTER VALUE **/

    //u3 = a3 * Fx + b3 * Fr + c3 * Fz
    vertical_thruster = coeff_matrix[2][0] * forward_force + coeff_matrix[2][1] *rotational_force  + coeff_matrix[2][2] * z_force;

    //check for saturation of the thruster for both positive and negative values
    vertical_thruster = (vertical_thruster < max_thruster_value) ? vertical_thruster: max_thruster_value;
    vertical_thruster = (vertical_thruster > -max_thruster_value) ? vertical_thruster : -max_thruster_value;


  /** Notify the output variables **/
    Notify("side_thruster_one", side_thruster_one);
    Notify("side_thruster_two", side_thruster_two);
    Notify("vertical_thruster", vertical_thruster);


}//end of void Saucisse::CalcThrustersValues()
