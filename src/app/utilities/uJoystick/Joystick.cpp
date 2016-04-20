/************************************************************/
/*    FILE: Joystick.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Vincent Drevelle, Simon Rohou, Guilherme Schvarcz Franco
/*    DATE: 2015-2016
/************************************************************/

#include "Joystick.h"

Joystick::Joystick()
{
}

Joystick::~Joystick()
{
}

bool Joystick::OnStartUp()
{
    AppCastingMOOSApp::OnStartUp();
    setlocale(LC_ALL, "C");

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
        reportConfigWarning("No config block found for " + GetAppName());

    int inputId = -1;
    int inputType = -1;
    float ratio = 1.;
    float axis_min = -1.;
    float axis_max = 1.;
    ControlMode inputMode = Value;

    STRING_LIST::iterator p;
    sParams.reverse();
    for(p = sParams.begin() ; p != sParams.end() ; p++)
    {
        string orig  = *p;
        string line  = *p;
        string param = toupper(biteStringX(line, '='));
        string value = line;
        bool handled = false;

        if(param == "DEVICE_NAME")
        {
            mJoystickControl.mDeviceName = value;
            handled = true;
        }

        else if(param == "MODE")
        {
            if (tolower(value) == "value")
                inputMode = Value;
            else if (tolower(value) == "increment")
                inputMode = Increment;
            else if (tolower(value) == "switch")
                inputMode = Switch;
            handled = true;
        }
        else if(param == "MIN")
        {
            if(!MOOSIsNumeric(value))
                reportConfigWarning(orig);
            else
                axis_min = atof(value.c_str());

            handled = true;
        }
        else if(param == "MAX")
        {
            if(!MOOSIsNumeric(value))
                reportConfigWarning(orig);
            else
                axis_max = atof(value.c_str());

            handled = true;
        }
        else if(param == "BUTTON")
        {
            if(!MOOSIsNumeric(value))
                reportConfigWarning(orig);
            else
            {
                inputId = atoi(value.c_str());
                inputType = JS_EVENT_BUTTON;
            }

            handled = true;
        }
        else if(param == "AXIS")
        {
            if(!MOOSIsNumeric(value))
                reportConfigWarning(orig);
            else
            {
                inputId = atoi(value.c_str());
                inputType = JS_EVENT_AXIS;
            }

            handled = true;
        }
        else if(param == "SCALE")
        {
            if(!MOOSIsNumeric(value))
                reportConfigWarning(orig);
            else
            {
                axis_max = atof(value.c_str());
                axis_min = -axis_max;
            }

            handled = true;
        }
        else if(param == "SCALE_RATIO")
        {
            if(!MOOSIsNumeric(value))
                reportConfigWarning(orig);

            else
                ratio = atof(value.c_str());

            handled = true;
        }
        else if(param == "MOOS_DEST_VAR")
        {
            value = toupper(value);

            if(inputId >= 0)
            {
                mControlsVariables[make_pair(inputType, inputId)] = value;

                double offset = ratio*axis_min;
                double gain = (ratio*axis_max - offset);

                mJoystickControl.setState(inputType, inputId, inputMode, gain, offset);
                handled = true;
            }
        }

        if(!handled)
            reportUnhandledConfigWarning(orig);
    }

    if(!mJoystickControl.OpenPort())
        MOOSFail("Cannot open the joystick device '%s'.\n", mJoystickControl.mDeviceName.c_str());

    registerVariables();
    return(true);
}

void Joystick::registerVariables()
{
    AppCastingMOOSApp::RegisterVariables();
}

bool Joystick::OnConnectToServer()
{
    registerVariables();
    return(true);
}

bool Joystick::OnNewMail(MOOSMSG_LIST &NewMail)
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

        if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }

    return(true);
}

bool Joystick::Iterate()
{
    AppCastingMOOSApp::Iterate();


    mJoystickControl.deviceReadLoop();

    map<pair<int/*type*/,int/*number*/>, string>::const_iterator it;
    for(it = mControlsVariables.begin(); it != mControlsVariables.end(); it++)
    {
        JoystickControlState state = mJoystickControl.mControlsState[it->first];
        switch (state.mode)
        {
        case Value:
        case Increment:
            Notify(it->second, state.value);
            break;
        case Switch:
            Notify(it->second, state.switchValue);
            break;
        }
    }

    AppCastingMOOSApp::PostReport();
    return(true);
}

bool Joystick::buildReport() 
{
#if 0 // Keep these around just for template
    m_msgs << "============================================ \n";
    m_msgs << "File:                                        \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
#endif

    m_msgs << mJoystickControl.toString() << "\n\n";

    return true;
}
