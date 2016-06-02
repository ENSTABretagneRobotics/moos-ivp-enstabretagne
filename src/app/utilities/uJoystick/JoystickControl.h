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

class JoystickControlState
{
public:
    ControlMode mode;
    bool switchValue;
    double value, increment, offset, gain;
    JoystickControlState():
        value(0), increment(0), offset(0), gain(1),
        switchValue(false), mode(Value)
    {}

    JoystickControlState(const JoystickControlState &copyFrom)
    {
        this->mode = copyFrom.mode;
        this->switchValue = copyFrom.switchValue;
        this->value = copyFrom.value;
        this->increment = copyFrom.increment;
        this->offset = copyFrom.offset;
        this->gain = copyFrom.gain;
    }
    bool operator ==(JoystickControlState b)
    {
        if (this->mode != b.mode)
            return false;
        switch (this->mode)
        {
        case Value:
        case Increment:
            return this->value == b.value;
        case Switch:
            return this->switchValue == b.switchValue;
        }

        return false;
    }

};

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
