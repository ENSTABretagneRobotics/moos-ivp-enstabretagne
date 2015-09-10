/************************************************************/
/*    FILE: RazorIMU.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "RazorIMU.h"

using namespace std;
using namespace std::placeholders; // needed for bind(x, x, _1)

//---------------------------------------------------------
// Constructor

RazorIMU::RazorIMU()
{
  m_razor_imu = NULL;
  m_razor_mode = RazorAHRS::ACC_MAG_GYR_RAW;

  m_yaw = m_pitch = m_roll = 0.;
  m_mag_x = m_mag_y = m_mag_z = 0.;
  m_acc_x = m_acc_y = m_acc_z = 0.;
  m_gyr_x = m_gyr_y = m_gyr_z = 0.;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool RazorIMU::OnNewMail(MOOSMSG_LIST &NewMail)
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

  return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool RazorIMU::OnConnectToServer()
{
  registerVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool RazorIMU::Iterate()
{
  AppCastingMOOSApp::Iterate();
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool RazorIMU::OnStartUp()
{
  bool init = true;
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if (!m_MissionReader.GetValue("RAZORIMU_SERIAL_PORT",m_serial_port))
    reportConfigWarning("No RAZORIMU_SERIAL_PORT config found for " + GetAppName());
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

    if(param == "MODE")
    {
      value = toupper(value);

      if(value == "YAW_PITCH_ROLL")
        m_razor_mode = RazorAHRS::YAW_PITCH_ROLL;

      else if(value == "ACC_MAG_GYR_RAW")
        m_razor_mode = RazorAHRS::ACC_MAG_GYR_RAW;

      else if(value == "ACC_MAG_GYR_CALIBRATED")
        m_razor_mode = RazorAHRS::ACC_MAG_GYR_CALIBRATED;

      else
        reportConfigWarning(orig);

      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  string msg;
  if(!initRazorIMU(msg))
  {
    reportConfigWarning("Razor initialization failed: " + msg
                        + "\nCheck serial port in .moos file."
                        + "\nYou need R/W access to that port.");
    init = false;
  }

  registerVariables();
  AppCastingMOOSApp::PostReport();
  return init;
}

//---------------------------------------------------------
// Procedure: registerVariables

void RazorIMU::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
}

//------------------------------------------------------------
// Procedure: buildReport()

bool RazorIMU::buildReport()
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

  return(true);
}

//------------------------------------------------------------
// Procedure: initRazorIMU()

bool RazorIMU::initRazorIMU(string &error_msg)
{
  bool init = true;

  try
  {
    m_razor_imu = new RazorAHRS(
                    m_serial_port,
                    bind(&RazorIMU::onRazorData, this, _1),
                    bind(&RazorIMU::onRazorError, this, _1),
                    m_razor_mode);
  }

  catch(runtime_error &e)
  {
    error_msg = string(e.what());
    init = false;
  }

  return init;
}

//------------------------------------------------------------
// Procedure: onRazorData()

void RazorIMU::onRazorData(const float data[])
{
  switch(m_razor_mode)
  {
    case RazorAHRS::YAW_PITCH_ROLL:
      m_yaw = data[0];
      m_pitch = data[2];
      m_roll = data[1];
      break;

    case RazorAHRS::ACC_MAG_GYR_RAW:
    case RazorAHRS::ACC_MAG_GYR_CALIBRATED:
      m_mag_x = data[4];
      m_mag_y = data[3];
      m_mag_z = data[5];
      m_acc_x = data[1];
      m_acc_y = data[0];
      m_acc_z = data[2];
      m_gyr_x = data[7];
      m_gyr_y = data[6];
      m_gyr_z = data[8];
      break;
  }

  notifyInertialData();
}

//------------------------------------------------------------
// Procedure: onRazorError()

void RazorIMU::onRazorError(const string &msg)
{
  reportConfigWarning("Razor data error: " + msg);
}

//------------------------------------------------------------
// Procedure: notifyInertialData()

void RazorIMU::notifyInertialData()
{
  switch(m_razor_mode)
  {
    case RazorAHRS::YAW_PITCH_ROLL:
      Notify("IMU_YAW", m_yaw);
      Notify("IMU_PITCH", m_pitch);
      Notify("IMU_ROLL", m_roll);
      break;

    case RazorAHRS::ACC_MAG_GYR_RAW:
    case RazorAHRS::ACC_MAG_GYR_CALIBRATED:
      Notify("IMU_ACC_X", m_acc_x);
      Notify("IMU_ACC_Y", m_acc_y);
      Notify("IMU_ACC_Z", m_acc_z);

      Notify("IMU_MAG_X", m_mag_x);
      Notify("IMU_MAG_Y", m_mag_y);
      Notify("IMU_MAG_Z", m_mag_z);

      Notify("IMU_GYR_X", m_gyr_x);
      Notify("IMU_GYR_Y", m_gyr_y);
      Notify("IMU_GYR_Z", m_gyr_z);
      break;
  }
}
