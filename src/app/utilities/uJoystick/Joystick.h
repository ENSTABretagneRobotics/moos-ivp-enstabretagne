/************************************************************/
/*    FILE: Joystick.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Vincent Drevelle, Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef Joystick_HEADER
#define Joystick_HEADER

#include <map>
#include <string>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class Joystick : public AppCastingMOOSApp
{
  public:
    Joystick();
    ~Joystick();

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp function to overload 
    bool buildReport();
    void registerVariables();

  protected: 
    bool deviceReadLoop();

  private:
    static bool read_loop_thread_function(void *ptr_this);

  private: // Configuration variables
    std::string m_device_name;
    std::map<int/*axis_id*/, std::string> m_axis_variables;
    std::map<int/*button_id*/, std::string> m_button_variables;
    std::map<std::string, double> m_variables;
    std::map<std::string, double> m_increment_variable;
    std::map<std::pair<int/*type*/,int/*number*/>, double> m_axis_offset;
    std::map<std::pair<int/*type*/,int/*number*/>, double> m_axis_gain;

  private: // State variables
    int m_file_device;
    CMOOSThread m_deviceReadThread;
};

#endif 
