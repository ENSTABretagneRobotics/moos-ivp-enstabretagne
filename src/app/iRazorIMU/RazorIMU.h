/************************************************************/
/*    FILE: RazorIMU.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef RazorIMU_HEADER
#define RazorIMU_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "razor-9dof-ahrs/RazorAHRS.h"

class RazorIMU : public AppCastingMOOSApp
{
  public:
    RazorIMU();
    ~RazorIMU() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // RazorIMU functions
    bool initRazorIMU(std::string &error_msg);
    void onRazorData(const float data[]);
    void onRazorError(const std::string &msg);
    void notifyInertialData();

  private: // Configuration variables
    std::string m_serial_port;
    RazorAHRS::Mode m_razor_mode;

  private: // State variables
    RazorAHRS *m_razor_imu;
    float m_yaw, m_pitch, m_roll;
    float m_mag_x, m_mag_y, m_mag_z;
    float m_acc_x, m_acc_y, m_acc_z;
    float m_gyr_x, m_gyr_y, m_gyr_z;
};

#endif