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
  m_yaw_declination = 0.0;
}

XSensINS::~XSensINS() {
  m_device.close();
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
  XsByteArray data;
  XsMessageArray msgs;
  m_device.readDataToBuffer(data);
  m_device.processBufferedData(data, msgs);

  for (XsMessageArray::iterator it = msgs.begin(); it != msgs.end(); ++it) {
    // Retrieve a packet
    XsDataPacket packet;
    packet.setMessage((*it));

    // Convert packet to Euler
    if(packet.containsOrientation()){
      m_euler = packet.orientationEuler();
      Notify("IMU_PITCH", -m_euler.pitch());
      Notify("IMU_ROLL", m_euler.roll());
      Notify("IMU_YAW", -m_euler.yaw() + m_yaw_declination);
    }

    // Acceleration
    if(packet.containsCalibratedAcceleration()){
      m_acceleration = packet.calibratedAcceleration();
      Notify("IMU_ACC_X", m_acceleration[0]);
      Notify("IMU_ACC_Y", m_acceleration[1]);
      Notify("IMU_ACC_Z", m_acceleration[2]);
    }

    //Gyro
    if(packet.containsCalibratedGyroscopeData()){
      m_gyro = packet.calibratedGyroscopeData();
      Notify("IMU_GYR_X", m_gyro[0]);
      Notify("IMU_GYR_Y", m_gyro[1]);
      Notify("IMU_GYR_Z", m_gyro[2]);
    }

    //Magneto
    if(packet.containsCalibratedMagneticField()){
      m_mag = packet.calibratedMagneticField();
      Notify("IMU_MAG_X", m_mag[0]);
      Notify("IMU_MAG_Y", m_mag[1]);
      Notify("IMU_MAG_Z", m_mag[2]);
      Notify("IMU_MAG_N", sqrt(pow(m_mag[0], 2) + pow(m_mag[1], 2) + pow(m_mag[2], 2)));
    }

    //LatLon
    if(packet.containsLatitudeLongitude()){
      m_latlon = packet.latitudeLongitude();
      Notify("INS_LAT", m_latlon[0]);
      Notify("INS_LONG", m_latlon[1]);
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
  if (!m_MissionReader.GetValue("XSENSINS_SERIAL_PORT",m_uart_port))
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
    if (param == "m_uart_baud_rate") {
      m_uart_baud_rate = atoi(value.c_str());
      handled = true;
    }
    else if (param == "YAW_DECLINATION") {
      m_yaw_declination = atoi(value.c_str());
      handled = true;
    }
    if(!handled)
      reportUnhandledConfigWarning(orig);
  }
  
  registerVariables();
  
  //------ OPEN INS ---------------//
  XsPortInfo mtPort("/dev/xsens", XsBaud::numericToRate(115200));
  if (!m_device.openPort(mtPort)) {
    reportRunWarning("Could not open the COM port" + m_uart_port);
  }

  //------ CONFIGURE INS ---------------//
  // Put the device into configuration mode before configuring the device
  if (!m_device.gotoConfig()) {
    reportRunWarning("Could not begin the config mode");
  }

  XsOutputConfiguration euler(XDI_EulerAngles, 25);
  XsOutputConfiguration acceleration(XDI_Acceleration, 25);
  XsOutputConfiguration rateOfTurn(XDI_RateOfTurn, 25);
  XsOutputConfiguration magnetic(XDI_MagneticField, 25);
  XsOutputConfiguration latlon(XDI_LatLon, 25);

  XsOutputConfigurationArray configArray;
  configArray.push_back(euler);
  configArray.push_back(acceleration);
  configArray.push_back(rateOfTurn);
  configArray.push_back(magnetic);
  configArray.push_back(latlon);
  // Save INS Config
  if (!m_device.setOutputConfiguration(configArray)) {
    reportRunWarning("Could not save config");
  }

  //------ START INS ---------------//
  if (!m_device.gotoMeasurement()) {
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
  actab << m_uart_port << m_uart_baud_rate << m_euler.yaw() + m_yaw_declination << m_euler.roll() << m_euler.pitch();
  m_msgs << actab.getFormattedString();

  return true;
}
