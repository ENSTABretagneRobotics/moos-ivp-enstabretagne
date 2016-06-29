/************************************************************/
/*    FILE: Joystick.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Vincent Drevelle, Simon Rohou, Guilherme Schvarcz Franco
/*    DATE: 2015-2016
/************************************************************/

#ifndef Joystick_HEADER
#define Joystick_HEADER

#include "MBUtils.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "JoystickControl.h"

using namespace std;

class Joystick : public AppCastingMOOSApp
{
  public:
    Joystick();
    ~Joystick();

  protected: // Standard MOOSApp functions to overload  
    bool OnStartUp();
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool buildReport();
    void registerVariables();

  private:
    JoystickControl mJoystickControl;
    map<pair<int/*type*/,int/*number*/>, pair<string, JoystickControlState> > mControlsVariables;

};

#endif 
