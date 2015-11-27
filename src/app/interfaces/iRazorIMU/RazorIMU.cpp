/************************************************************/
/*    FILE: RazorIMU.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
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
    else if (param == "SERIAL_PORT")
    {
      m_serial_port = value.c_str();
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  if(!initRazorIMU(m_init_razor_msg))
  {
    reportConfigWarning("Razor initialization failed: " + m_init_razor_msg
                        + "\nCheck serial port in .moos file."
                        + "\nYou need R/W access to that port.");
    init = false;
  }

  registerVariables();
  AppCastingMOOSApp::PostReport();
  return true;
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
  m_msgs << "Serial port: " << m_serial_port << "\n";
  m_msgs << "Init razor: " << m_init_razor_msg << "\n";
  m_msgs << "Razor IMU mode: ";

  if(m_razor_mode == RazorAHRS::YAW_PITCH_ROLL)
    m_msgs << "YAW_PITCH_ROLL";

  else if(m_razor_mode == RazorAHRS::ACC_MAG_GYR_RAW)
    m_msgs << "ACC_MAG_GYR_RAW";

  else if(m_razor_mode == RazorAHRS::ACC_MAG_GYR_CALIBRATED)
    m_msgs << "ACC_MAG_GYR_CALIBRATED";

  m_msgs << "\n\n";

  ACTable actab_euler(3);
  actab_euler << "Yaw | Pitch | Roll";
  actab_euler.addHeaderLines();
  if(m_razor_mode == RazorAHRS::YAW_PITCH_ROLL)
    actab_euler << m_yaw << m_pitch << m_roll;
  else
    actab_euler << "-" << "-" << "-";
  m_msgs << actab_euler.getFormattedString();
  m_msgs << "\n\n";

  ACTable actab_acc(3);
  actab_acc << "Acc X | Acc Y | Acc Z";
  actab_acc.addHeaderLines();
  if(m_razor_mode == RazorAHRS::ACC_MAG_GYR_RAW
    || m_razor_mode == RazorAHRS::ACC_MAG_GYR_CALIBRATED)
    actab_acc << m_acc_x << m_acc_y << m_acc_z;
  else
    actab_acc << "-" << "-" << "-";
  m_msgs << actab_acc.getFormattedString();
  m_msgs << "\n\n";

  ACTable actab_mag(3);
  actab_mag << "Mag X | Mag Y | Mag Z";
  actab_mag.addHeaderLines();
  if(m_razor_mode == RazorAHRS::ACC_MAG_GYR_RAW
    || m_razor_mode == RazorAHRS::ACC_MAG_GYR_CALIBRATED)
    actab_mag << m_mag_x << m_mag_y << m_mag_z;
  else
    actab_mag << "-" << "-" << "-";
  m_msgs << actab_mag.getFormattedString();
  m_msgs << "\n\n";

  ACTable actab_gyr(3);
  actab_gyr << "Gyr X | Gyr Y | Gyr Z";
  actab_gyr.addHeaderLines();
  if(m_razor_mode == RazorAHRS::ACC_MAG_GYR_RAW
    || m_razor_mode == RazorAHRS::ACC_MAG_GYR_CALIBRATED)
    actab_gyr << m_gyr_x << m_gyr_y << m_gyr_z;
  else
    actab_gyr << "-" << "-" << "-";
  m_msgs << actab_gyr.getFormattedString();
  m_msgs << "\n\n";

  return(true);
}

//------------------------------------------------------------
// Procedure: initRazorIMU()

bool RazorIMU::initRazorIMU(string &init_msg)
{
  bool init = true;

  try
  {
    m_razor_imu = new RazorAHRS(
                    m_serial_port,
                    bind(&RazorIMU::onRazorData, this, _1),
                    bind(&RazorIMU::onRazorError, this, _1),
                    m_razor_mode);
    init_msg = "ok";
  }

  catch(runtime_error &e)
  {
    init_msg = string(e.what());
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
      Notify("RAZOR_YAW", m_yaw);
      Notify("RAZOR_PITCH", m_pitch);
      Notify("RAZOR_ROLL", m_roll);
      break;

    case RazorAHRS::ACC_MAG_GYR_RAW:
    case RazorAHRS::ACC_MAG_GYR_CALIBRATED:
      Notify("RAZOR_ACC_X", m_acc_x);
      Notify("RAZOR_ACC_Y", m_acc_y);
      Notify("RAZOR_ACC_Z", m_acc_z);

      Notify("RAZOR_MAG_X", m_mag_x);
      Notify("RAZOR_MAG_Y", m_mag_y);
      Notify("RAZOR_MAG_Z", m_mag_z);

      Notify("RAZOR_GYR_X", m_gyr_x);
      Notify("RAZOR_GYR_Y", m_gyr_y);
      Notify("RAZOR_GYR_Z", m_gyr_z);
      break;
  }
}
