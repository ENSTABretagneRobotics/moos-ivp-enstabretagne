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
    else if(key == "FR_ROTATIONAL_FORCE" || key == "FZ_VERTICAL_FORCE" || key == "FX_FORWARD_FORCE")
    {

      if(OPERATION_MODE == "AUTONOMUS" || OPERATION_MODE == "SEMI-AUTONOMUS")
      {

        //update the force value
        if(key == "FR_ROTATIONAL_FORCE")
          FR_ROTATIONAL_FORCE =  msg.GetDouble();
        else if(key == "FX_FORWARD_FORCE")
          FX_FORWARD_FORCE =  msg.GetDouble();
        else if(key == "FZ_VERTICAL_FORCE")
          FZ_VERTICAL_FORCE =  msg.GetDouble();

        //execute the calculation of the Thurster Values;
        Saucisse::CalcThrustersValues();

      }//end of if operation mode
      else if(OPERATION_MODE == "DEPTH_CONTROL_ONLY")
      {
          //update the force value
        if(key == "FZ_VERTICAL_FORCE")
          FZ_VERTICAL_FORCE =  msg.GetDouble();

        //execute the calculation of the Thurster Values;
        Saucisse::CalcThrustersValues();
      
      }//end of if operation mode

    }

    /** JOYSTICK FORCE VALUES UPDATE**/
    else if(key == "DESIRED_ELEVATOR" || key == "DESIRED_THRUST" || key == "DESIRED_RUDDER")
    {

      //we update the forces from joystick only o manual mode
      if(OPERATION_MODE == "MANUAL")
      {
          //update the force value
        if(key == "DESIRED_RUDDER")
          FR_ROTATIONAL_FORCE =  msg.GetDouble();
        else if(key == "DESIRED_THRUST")
          FX_FORWARD_FORCE =  msg.GetDouble();
        else if(key == "DESIRED_ELEVATOR")
          FZ_VERTICAL_FORCE =  msg.GetDouble();

        //execute the calculation of the Thurster Values;
        Saucisse::CalcThrustersValues();
      
      }//end of if operation mode
      else if(OPERATION_MODE == "DEPTH_CONTROL_ONLY")
      {
          //update the force value
        if(key == "DESIRED_RUDDER")
          FR_ROTATIONAL_FORCE =  msg.GetDouble();
        else if(key == "DESIRED_THRUST")
          FX_FORWARD_FORCE =  msg.GetDouble();

        //execute the calculation of the Thurster Values;
        Saucisse::CalcThrustersValues();
      
      }//end of if operation mode
      
    }

    else if(key == "MANUAL_MODE_BUTTON") 
    {
      //toggle operation mode between MANUAL and SEMI-AUTONOMUS
      if(msg.GetDouble() < 0)
      {
        OPERATION_MODE = "MANUAL";
        Notify("OPERATION_MODE", OPERATION_MODE);
      }
    }  

    else if(key == "SEMI_AUTONOMUS_MODE_BUTTON") 
    {
      //toggle operation mode between MANUAL and SEMI-AUTONOMUS
      if(msg.GetDouble() < 0)
      {
        OPERATION_MODE = "SEMI-AUTONOMUS";
        Notify("OPERATION_MODE", OPERATION_MODE);
      }

    }  

    else if(key == "DEPTH_CONTROL_MODE_BUTTON") 
    {
      //put the operation mode to DEPTH_CONTROL_ONLY
      if(msg.GetDouble() < 0)
      {
        OPERATION_MODE = "DEPTH_CONTROL_ONLY";
        Notify("OPERATION_MODE", OPERATION_MODE);
      }

    }


    

    else if(key == "COEFF_MATRIX")
    {
      
      vector<string> str_vector = parseString(msg.GetString(), ',');
      
      //check for correct array size of 9 elements (3x3)
      if(str_vector.size() == 9)
      {
        unsigned int i = 0; //index for the str_vector

            for(unsigned int j=0; j<3; j++)
            {
              for(unsigned int k=0; k<3; k++)
              {
                //get the string and convert it into value
                COEFF_MATRIX[j][k] = atof(str_vector[i].c_str());
                i++;
              }//end of for k

            }//end of for j

      }//end of if vector size

    }//end of else if


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
  COEFF_MATRIX[0][0] = 1;
  COEFF_MATRIX[0][1] = 1;
  COEFF_MATRIX[0][2] = 0;
  COEFF_MATRIX[1][0] = 1;
  COEFF_MATRIX[1][1] = -1;
  COEFF_MATRIX[1][2] = 0;
  COEFF_MATRIX[2][0] = 0;
  COEFF_MATRIX[2][1] = 0;
  COEFF_MATRIX[2][2] = 1;

  max_thruster_value = 1; //[-1;1]

  //zero value means thursters stopped
  U1_SIDE_THRUSTER_ONE = 0; 
  U2_SIDE_THRUSTER_TWO = 0;
  U3_VERTICAL_THRUSTER = 0;

  /** variable initialization **/
  FR_ROTATIONAL_FORCE = 0;     
  FZ_VERTICAL_FORCE = 0;
  FX_FORWARD_FORCE = 0;
  saturation_value = 0;
  saturated_thruster_value = 0;
  OPERATION_MODE = "MANUAL";

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
    else if(param == "MAX_THRUSTER_VALUE")
    {
      max_thruster_value = atof(value.c_str());
      handled = true;
    }
    else if(param == "COEFF_MATRIX")
    {
      
      vector<string> str_vector = parseString(value, ',');
      
      //check for correct array size of 9 elements (3x3)
      if(str_vector.size() == 9)
      {
        unsigned int i = 0;
            for(unsigned int j=0; j<3; j++)
            {
              for(unsigned int k=0; k<3; k++)
              {
                //get the string and convert it into value
                COEFF_MATRIX[j][k] = atof(str_vector[i].c_str());
                i++;
              }
            }
      }
      else
      {
        //incorrect array size
        //send warning
        reportUnhandledConfigWarning("Error while parsing COEFF_MATRIX: incorrect number of elements");

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

  Register("COEFF_MATRIX", 0);
  
  Register("FR_ROTATIONAL_FORCE", 0);
  Register("FZ_VERTICAL_FORCE", 0);
  Register("FX_FORWARD_FORCE", 0);

  Register("DESIRED_RUDDER", 0);
  Register("DESIRED_THRUST", 0);
  Register("DESIRED_ELEVATOR", 0);

  Register("MANUAL_MODE_BUTTON", 0);
  Register("SEMI_AUTONOMUS_MODE_BUTTON", 0);
  Register("DEPTH_CONTROL_MODE_BUTTON", 0);


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
  m_msgs << "\n";
  m_msgs << "OPERATION_MODE: " << OPERATION_MODE;
  m_msgs << "\n";
  m_msgs << "\n";


  m_msgs << "=========================== \n";
  m_msgs << "Coefficient matrix:         \n";
  m_msgs << "\n";
  m_msgs << "     Fx   Fr   Fz \n";
  m_msgs << " ----------------- \n";
  m_msgs << "u1 |  "<< COEFF_MATRIX[0][0] <<"   "<< COEFF_MATRIX[0][1] <<"    "<< COEFF_MATRIX[0][2] <<"  \n";
  m_msgs << "u2 |  "<< COEFF_MATRIX[1][0] <<"   "<< COEFF_MATRIX[1][1] <<"    "<< COEFF_MATRIX[1][2] <<"  \n";
  m_msgs << "u3 |  "<< COEFF_MATRIX[2][0] <<"   "<< COEFF_MATRIX[2][1] <<"    "<< COEFF_MATRIX[2][2] <<"  \n";
  m_msgs << "\n";

  m_msgs << "=========================== \n";
  m_msgs << "Desired forces:         \n";
  m_msgs << "Fx =  "<< FX_FORWARD_FORCE <<" \n";
  m_msgs << "Fr =  "<< FR_ROTATIONAL_FORCE <<"\n";
  m_msgs << "Fz =  "<< FZ_VERTICAL_FORCE <<" \n";
  m_msgs << "\n";

  m_msgs << "=========================== \n";
  m_msgs << "Thrusters values:         \n";
  m_msgs << "u1 =  "<< U1_SIDE_THRUSTER_ONE <<" \n";
  m_msgs << "u2 =  "<< U2_SIDE_THRUSTER_TWO <<"\n";
  m_msgs << "u3 =  "<< U3_VERTICAL_THRUSTER <<" \n";




  return true;
}


void Saucisse::CalcThrustersValues()
{

  /** COEFFICIENT MATRIX 

    double COEFF_MATRIX = [3][3]

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
  U1_SIDE_THRUSTER_ONE = COEFF_MATRIX[0][0] * FX_FORWARD_FORCE + COEFF_MATRIX[0][1] * FR_ROTATIONAL_FORCE + COEFF_MATRIX[0][2] * FZ_VERTICAL_FORCE;

  //u2 = a2 * Fx + b2 * Fr + c2 * Fz
  U2_SIDE_THRUSTER_TWO = COEFF_MATRIX[1][0] * FX_FORWARD_FORCE + COEFF_MATRIX[1][1] * FR_ROTATIONAL_FORCE + COEFF_MATRIX[1][2] * FZ_VERTICAL_FORCE;





  /** CHECK FOR SATURATION of the side thrusters **/
    if(fabs(U1_SIDE_THRUSTER_ONE) > max_thruster_value || fabs(U2_SIDE_THRUSTER_TWO) > max_thruster_value)
    {
      //one of the two thrusters are saturated, remove the saturation
      //find out which one of the thursters is saturated
      if(fabs(U1_SIDE_THRUSTER_ONE) > fabs(U2_SIDE_THRUSTER_TWO))
      {
        //thruster one is saturated
        saturated_thruster_value = U1_SIDE_THRUSTER_ONE;
      
      }//end of if
      else
      {
        //thruster two is saturated
        saturated_thruster_value = U2_SIDE_THRUSTER_TWO;
        
      }//end of else

      //calculate the saturation value, which will be removed from the thursters values
      saturation_value = fabs(saturated_thruster_value) - max_thruster_value;

      //check if the saturation is in (a)forward or (b)reverse direction
      if(saturated_thruster_value > 0)
      {
        //(a) saturation is in forward direction (positive) so we must REDUCE the saturated value
        U1_SIDE_THRUSTER_ONE = U1_SIDE_THRUSTER_ONE - saturation_value;
        U2_SIDE_THRUSTER_TWO = U2_SIDE_THRUSTER_TWO - saturation_value;
        //from now we have non saturated value for both thrusters
      }
      else
      {
        //(b) saturation is in reverse direction (negative) so we must SUM the saturated value in order to "desaturate" the thrusters output values
        U1_SIDE_THRUSTER_ONE = U1_SIDE_THRUSTER_ONE + saturation_value;
        U2_SIDE_THRUSTER_TWO = U2_SIDE_THRUSTER_TWO + saturation_value;
        //from now we have non saturated value for both thrusters
      }



      //check for the limits of the output thrusters values
      U1_SIDE_THRUSTER_ONE = (U1_SIDE_THRUSTER_ONE < max_thruster_value) ? U1_SIDE_THRUSTER_ONE: max_thruster_value;
      U1_SIDE_THRUSTER_ONE = (U1_SIDE_THRUSTER_ONE > -max_thruster_value) ? U1_SIDE_THRUSTER_ONE : -max_thruster_value;

      U2_SIDE_THRUSTER_TWO = (U2_SIDE_THRUSTER_TWO < max_thruster_value) ? U2_SIDE_THRUSTER_TWO: max_thruster_value;
      U2_SIDE_THRUSTER_TWO = (U2_SIDE_THRUSTER_TWO > -max_thruster_value) ? U2_SIDE_THRUSTER_TWO : -max_thruster_value;

    }//end of if saturation check
    //else - the thrusters values are not saturated so they can be sent to the thursters whitout additional manipulation




  /** END OF CHECK FOR SATURATION of the side thrusters **/




  /** CALCULATION OF THE VERTICAL THRUSTER VALUE **/

    //u3 = a3 * Fx + b3 * Fr + c3 * Fz
    U3_VERTICAL_THRUSTER = COEFF_MATRIX[2][0] * FX_FORWARD_FORCE + COEFF_MATRIX[2][1] * FR_ROTATIONAL_FORCE  + COEFF_MATRIX[2][2] * FZ_VERTICAL_FORCE;

    //check for saturation of the thruster for both positive and negative values
    U3_VERTICAL_THRUSTER = (U3_VERTICAL_THRUSTER < max_thruster_value) ? U3_VERTICAL_THRUSTER: max_thruster_value;
    U3_VERTICAL_THRUSTER = (U3_VERTICAL_THRUSTER > -max_thruster_value) ? U3_VERTICAL_THRUSTER : -max_thruster_value;


  /** Notify the output variables **/
    Notify("U1_SIDE_THRUSTER_ONE", U1_SIDE_THRUSTER_ONE);
    Notify("U2_SIDE_THRUSTER_TWO", U2_SIDE_THRUSTER_TWO);
    Notify("U3_VERTICAL_THRUSTER", U3_VERTICAL_THRUSTER);


}//end of void Saucisse::CalcThrustersValues()
