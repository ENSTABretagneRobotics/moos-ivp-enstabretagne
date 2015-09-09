/************************************************************/
/*    FILE: XSensINS.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH:
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "XSensINS.h"

#include <math.h>

using namespace std;

//---------------------------------------------------------
// Constructor

XSensINS::XSensINS() {
  yaw_declination = 0.0;
}

XSensINS::~XSensINS() {
  device.close();
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool XSensINS::OnNewMail(MOOSMSG_LIST &NewMail) {
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for (p = NewMail.begin() ; p != NewMail.end() ; p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

    #if 0  // Keep these around just for template
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString();
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif

    if (key == "FOO")
      cout << "great!";

    else if (key != "APPCAST_REQ")  // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool XSensINS::OnConnectToServer() {
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool XSensINS::Iterate() {
  AppCastingMOOSApp::Iterate();

  // Read Data
  device.readDataToBuffer(data);
  device.processBufferedData(data, msgs);

  for (XsMessageArray::iterator it = msgs.begin(); it != msgs.end(); ++it) {
    // Retrieve a packet
    XsDataPacket packet;
    packet.setMessage((*it));

    // Convert packet to euler
    if(packet.containsOrientation()){
      euler = packet.orientationEuler();
      Notify("IMU_PITCH", euler.m_pitch);
      Notify("IMU_ROLL", euler.m_roll);
      Notify("IMU_YAW", euler.m_yaw + yaw_declination);
    }

    // Acceleration
    if(packet.containsCalibratedAcceleration()){
      acceleration = packet.calibratedAcceleration();
      Notify("IMU_ACC_X", acceleration[0]);
      Notify("IMU_ACC_Y", acceleration[1]);
      Notify("IMU_ACC_Z", acceleration[2]);
    }

    //Gyro
    if(packet.containsCalibratedGyroscopeData()){
      gyro = packet.calibratedGyroscopeData();
      Notify("IMU_GYR_X", gyro[0]);
      Notify("IMU_GYR_Y", gyro[1]);
      Notify("IMU_GYR_Z", gyro[2]);
    }

    //Magneto
    if(packet.containsCalibratedMagneticField()){
      mag = packet.calibratedMagneticField();
      Notify("IMU_MAG_X", mag[0]);
      Notify("IMU_MAG_Y", mag[1]);
      Notify("IMU_MAG_Z", mag[2]);
      Notify("IMU_MAG_N", sqrt(pow(mag[0], 2) + pow(mag[1], 2) + pow(mag[2], 2)));
    }

  }
  msgs.clear();

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool XSensINS::OnStartUp() {
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if (!m_MissionReader.GetValue("XSENSINS_SERIAL_PORT",UART_PORT))
    reportConfigWarning("No XSENSINS_SERIAL_PORT config found for " + GetAppName());
  if (!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  sParams.reverse();
  for (p = sParams.begin() ; p != sParams.end() ; p++) {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if (param == "UART_BAUD_RATE") {
      UART_BAUD_RATE = atoi(value.c_str());
      handled = true;
    }
    else if (param == "YAW_DECLINATION") {
      yaw_declination = atoi(value.c_str());
      handled = true;
    }
    if (!handled)
      reportUnhandledConfigWarning(orig);
  }
  registerVariables();

  //------ OPEN INS ---------------//
  XsPortInfo mtPort(UART_PORT, XsBaud::numericToRate(UART_BAUD_RATE));
  if (!device.openPort(mtPort)) {
    reportRunWarning("Could not open the COM port" + UART_PORT);
  }

  //------ CONFIGURE INS ---------------//
  // Put the device into configuration mode before configuring the device
  if (!device.gotoConfig()) {
    reportRunWarning("Could not begin the config mode");
  }

  XsOutputConfiguration euler(XDI_EulerAngles, 25);
  XsOutputConfiguration acceleration(XDI_Acceleration, 25);
  XsOutputConfiguration rateOfTurn(XDI_RateOfTurn, 25);
  XsOutputConfiguration magnetic(XDI_MagneticField, 25);

  XsOutputConfigurationArray configArray;
  configArray.push_back(euler);
  configArray.push_back(acceleration);
  configArray.push_back(rateOfTurn);
  configArray.push_back(magnetic);
  // Save INS Config
  if (!device.setOutputConfiguration(configArray)) {
    reportRunWarning("Could not save config");
  }

  //------ START INS ---------------//
  if (!device.gotoMeasurement()) {
    reportRunWarning("Could not start the INS");
  }

  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void XSensINS::registerVariables() {
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool XSensINS::buildReport() {

  m_msgs << "============================================ \n";
  m_msgs << "iXSensINS Status:                            \n";
  m_msgs << "============================================ \n";

  ACTable actab(5);
  actab << "Serial Port | Baude rate | YAW | ROLL | PITCH";
  actab.addHeaderLines();
  actab << UART_PORT << UART_BAUD_RATE << euler.m_yaw + yaw_declination << euler.m_roll << euler.m_pitch;
  m_msgs << actab.getFormattedString();

  return true;
}
