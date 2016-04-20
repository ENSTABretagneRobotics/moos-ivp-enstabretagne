/************************************************************/
/*    FILE: JoystickControl.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#ifndef JOYSTICKCONTROL_H
#define JOYSTICKCONTROL_H

#include <map>
#include <unistd.h>
#include <linux/joystick.h>
#include <sys/fcntl.h>
#include "ACTable.h"

using namespace std;
enum ControlMode
{
    Value, Increment, Switch
};

typedef struct
{
    ControlMode mode = Value;
    bool switchValue = false;
    double value = 0, increment = 0, offset = 0, gain = 1;

} JoystickControlState;

class JoystickControl
{
public:
    JoystickControl();
    ~JoystickControl();
    bool OpenPort();
    void ClosePort();
    void setState(int inputType, int inputNumber, ControlMode mode, double gain, double offset);

    map<pair<int/*type*/,int/*number*/>, JoystickControlState> mControlsState;
    string mDeviceName;
    bool deviceReadLoop();
    string toString();

protected:

private: // Configuration variables
  int mFileDevice;

};

#endif // JOYSTICKCONTROL_H
