/************************************************************/
/*    FILE: JoystickControl.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#include "JoystickControl.h"

JoystickControl::JoystickControl()
{
    mDeviceName = "/dev/input/js0";
    mFileDevice = -1;
}

JoystickControl::~JoystickControl()
{
    this->ClosePort();
}

bool JoystickControl::OpenPort()
{
    mFileDevice = open(mDeviceName.c_str(), O_RDONLY);
    return (mFileDevice != -1);
}

void JoystickControl::ClosePort()
{
    close(mFileDevice);
    mFileDevice = -1;
}

void JoystickControl::setState(int inputType, int inputNumber, ControlMode mode, double gain, double offset)
{
    JoystickControlState state = mControlsState[make_pair(inputType,inputNumber)];

    state.mode = mode;
    state.gain = gain;
    state.offset = offset;

    mControlsState[make_pair(inputType,inputNumber)] = state;
}

bool JoystickControl::deviceReadLoop()
{
    js_event e;
    fd_set fdset;
    timeval timeout = {0, 100000}; // sec, usec
    FD_ZERO(&fdset);
    FD_SET(mFileDevice, &fdset);

    if(select(mFileDevice + 1, &fdset, 0, 0, &timeout) > 0)
    {
        read(mFileDevice, &e, sizeof(js_event));

        // do not differentiate between synthetic int events and real events
        e.type &= ~JS_EVENT_INIT;

        JoystickControlState state = mControlsState[make_pair(e.type,e.number)];

        double adjustedValue = e.value;

        //Just to normalize in [0-1]
        if(e.type == JS_EVENT_AXIS)
            adjustedValue = (double)(-adjustedValue+32767)/65535.;

        adjustedValue = state.gain * adjustedValue + state.offset;
        switch(state.mode)
        {
        case Value:
            state.value = adjustedValue;
            break;
        case Increment:
            state.increment = adjustedValue;
            state.value += state.increment;
        break;
        case Switch:
            if (e.value == 1.)
            {
                state.switchValue  = !state.switchValue;
            }
            state.value = e.value;
        break;
        }

        mControlsState[make_pair(e.type,e.number)] = state;
    }

    return true;
}


string JoystickControl::toString()
{
    ACTable actab_button(8);
    actab_button << "Type" << "Id" << "Value" << "Actived" << "Increment" << "Offset" << "Gain" << "Mode";
    actab_button.addHeaderLines();

    for(map<pair<int/*type*/,int/*number*/>, JoystickControlState>::iterator it = mControlsState.begin() ;
        it != mControlsState.end();
        it++)
    {
        JoystickControlState state = it->second;

        if(it->first.first == JS_EVENT_AXIS)
            actab_button << "Axis";
        else
            actab_button << "Button";
        actab_button << it->first.second;

        actab_button << state.value;
        if(state.switchValue)
            actab_button << "true";
        else
            actab_button << "false";

        actab_button << state.increment
                     << state.offset
                     << state.gain;

        switch (state.mode) {
        case Value:
            actab_button << "Value";
            break;
        case Increment:
            actab_button << "Increment";
            break;
        case Switch:
            actab_button << "Switch";
            break;
        }
    }

    return actab_button.getFormattedString();

}
