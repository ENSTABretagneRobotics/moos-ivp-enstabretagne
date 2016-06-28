/************************************************************/
/*    FILE: SimplePID.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Thomas Le Mezo
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "math.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "SimplePID.h"
#include <math.h>

using namespace std;

//---------------------------------------------------------
// Constructor

SimplePID::SimplePID()
{
    m_consigne                    = 0.0;
    m_state                       = 0.0;
    m_state_previous              = 0.0;
    m_error                       = 0.0;
    m_command                     = 0.0;
    m_staturate                   = 0.0;

    m_state_differential          = 0.0;
    m_state_previous              = 0.0;

    m_kp                          = 0.0;
    m_ki                          = 0.0;
    m_kd                          = 0.0;
    m_kw                          = 0.0;

    m_dt                          = 0.0;
    m_I                           = 0.0;

    m_moosvar_state               = "STATE";
    m_moosvar_consigne            = "DESIRED_STATE";
    m_moosvar_command             = "COMMAND";
    m_moosvar_saturation          = "SATURATION";
    m_moosvar_state_diffferential = "";
    m_differential_input          = false;

    m_angle                = false;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SimplePID::OnNewMail(MOOSMSG_LIST &NewMail)
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

        if(key == m_moosvar_state)
            m_state = msg.GetDouble();
        else if(key == m_moosvar_consigne){
            m_consigne = msg.GetDouble();
            m_I = 0.0;
        }
        else if(key == m_moosvar_state_diffferential)
            m_state_differential = msg.GetDouble();
        else if(key == m_moosvar_saturation)
            m_staturate += msg.GetDouble();

        else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
            reportRunWarning("Unhandled Mail: " + key);
    }

    return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool SimplePID::OnConnectToServer()
{
    registerVariables();
    return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool SimplePID::Iterate()
{
    AppCastingMOOSApp::Iterate();

    m_dt = MOOSTime() - m_time_previous;
    m_time_previous = MOOSTime();

    // Proportional
    m_error = m_consigne - m_state;

    if(m_angle){
        m_error = 2.0*atan(tan(m_error/2.0));
    }

    double P = m_dt * m_kp * m_error;

    // Integral
    m_I += m_dt * m_ki * m_error;

    // Derivation
    if(m_differential_input==false){
        m_state_differential = m_state - m_state_previous;
    }
    double D = m_dt * m_kd * m_state_differential;

    // Anti-windup
    double W = m_kw * m_staturate;
    m_staturate = 0.0;

    m_command = P + m_I + D + W;
    // Notify command
    Notify(m_moosvar_command,m_command);
    m_state_previous = m_state;

    AppCastingMOOSApp::PostReport();
    return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SimplePID::OnStartUp()
{
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
        reportConfigWarning("No config block found for " + GetAppName());

    STRING_LIST::iterator p;
    sParams.reverse();
    for(p = sParams.begin() ; p != sParams.end() ; p++)
    {
        string orig  = *p;
        string line  = *p;
        string param = toupper(biteStringX(line, '='));
        string value = line;
        bool handled = false;

        if(param == "KP"){
            m_kp = atof(value.c_str()); handled = true;
        }
        else if(param == "KI"){
            m_ki = atof(value.c_str()); handled = true;
        }
        else if(param == "KD"){
            m_kd = atof(value.c_str()); handled = true;
        }
        else if(param == "KW"){
            m_kw = atof(value.c_str()); handled = true;
        }
        else if(param == "MOOSVAR_STATE"){
            m_moosvar_state = value;    handled = true;
        }
        else if(param == "MOOSVAR_CONSIGNE"){
            m_moosvar_consigne = value; handled = true;
        }
        else if(param == "MOOSVAR_COMMAND"){
            m_moosvar_command = value;  handled = true;
        }
        else if(param == "MOOSVAR_SATURATION"){
            m_moosvar_saturation = value;  handled = true;
        }
        else if(param == "MOOSVAR_STATE_DIFFFERENTIAL"){
            m_moosvar_state_diffferential = value;
            m_differential_input = true;
            handled = true;
        }
        else if(param == "ANGLE"){
            if(value == "RAD"){
                m_angle = true;
                handled = true;
            }
        }

        if(!handled)
            reportUnhandledConfigWarning(orig);
    }

    registerVariables();
    return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void SimplePID::registerVariables()
{
    AppCastingMOOSApp::RegisterVariables();
    Register(m_moosvar_consigne, 0);
    Register(m_moosvar_state, 0);

    if(m_differential_input == true){
        Register(m_moosvar_state_diffferential, 0);
    }
}

//------------------------------------------------------------
// Procedure: buildReport()

bool SimplePID::buildReport()
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

    ACTable actab(4);
    actab << "State | Consigne | Error | Command";
    actab.addHeaderLines();
    if(m_angle == false){
        actab << m_state << m_consigne << m_error;
    }
    else{
        actab << m_state*180.0/M_PI << m_consigne*180.0/M_PI << m_error*180.0/M_PI;
    }
     actab << m_command;


    m_msgs << actab.getFormattedString();
    m_msgs << '\n';

    ACTable actab2(4);
    actab2 << "Kp | Ki | Kd | Kz ";
    actab2.addHeaderLines();
    actab2 << m_kp << m_ki << m_kd << m_kw;
    m_msgs << actab2.getFormattedString();

    return true;
}
