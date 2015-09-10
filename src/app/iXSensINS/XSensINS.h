/************************************************************/
/*    FILE: XSensINS.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#ifndef XSensINS_HEADER
#define XSensINS_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#include <xsens/xsportinfoarray.h>
#include <xsens/xsdatapacket.h>
#include <xsens/xstime.h>
#include <xcommunication/legacydatapacket.h>
#include <xcommunication/int_xsdatapacket.h>
#include <xcommunication/enumerateusbdevices.h>

#include "deviceclass.h"

class XSensINS : public AppCastingMOOSApp
{
  public:
    XSensINS();
    ~XSensINS();

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // XSensINS functions


  private: // Configuration variables
    DeviceClass device;
    std::string UART_PORT;
    int UART_BAUD_RATE;

  private: // State variables
    XsByteArray data;
    XsMessageArray msgs;

    double yaw_declination; // magnetic declination

    XsEuler euler;
    XsVector acceleration;
    XsVector gyro;
    XsVector mag;

};

#endif 
