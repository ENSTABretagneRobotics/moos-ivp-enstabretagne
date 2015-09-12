/************************************************************/
/*    FILE: Joystick.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Vincent Drevelle, Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include <unistd.h>
#include <linux/joystick.h>
#include <sys/fcntl.h>
#include "MBUtils.h"
#include "ACTable.h"
#include "Joystick.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Joystick::Joystick()
{
  m_device_name = "/dev/input/js0";
  m_deviceReadThread.Initialise(read_loop_thread_function, (void*)this);
}

//---------------------------------------------------------
// Destructor

Joystick::~Joystick()
{
  m_deviceReadThread.Stop();
  close(m_file_device);
}

//---------------------------------------------------------
// Procedure: OnNewMail

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

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Joystick::OnConnectToServer()
{
  registerVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Joystick::Iterate()
{
  AppCastingMOOSApp::Iterate();
  
  map<string, double>::const_iterator it;

  // Update incremental variables
  for(it = m_increment_variable.begin() ; it != m_increment_variable.end() ; ++it)
    m_variables[it->first] += it->second;
  
  // Refresh variables  
  for(it = m_variables.begin() ; it != m_variables.end() ; ++it)
    Notify(it->first, it->second);

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Joystick::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
  setlocale(LC_ALL, "C");

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  int axis_id = -1;
  int button_id = -1;
  float ratio = 1.;
  float axis_min = -1.;
  float axis_max = 1.;
  bool increment = false;

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
      m_device_name = value;
      handled = true;
    }

    else if(param == "INCREMENT")
    {
      increment = tolower(value) == "true";
      handled = true;
    }

    else if(param == "SCALE")
    {
      if(!MOOSIsNumeric(value))
        reportConfigWarning(orig);

      else
      {
        axis_max = -atof(value.c_str());
        axis_min = -axis_max;
      }

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
        button_id = atoi(value.c_str());
        axis_id = -1;
      }

      handled = true;
    }

    else if(param == "AXIS")
    {
      if(!MOOSIsNumeric(value))
        reportConfigWarning(orig);

      else
      {
        axis_id = atoi(value.c_str());
        button_id = -1;
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

      if(button_id > 0)
      {
        m_button_variables[button_id] = value;
        m_axis_gain[make_pair(JS_EVENT_BUTTON, button_id)] = ratio*axis_max - ratio*axis_min;
        m_axis_offset[make_pair(JS_EVENT_BUTTON, button_id)] = ratio*axis_min;
      }

      else
      {
        m_axis_variables[axis_id] = value;
        m_axis_gain[make_pair(JS_EVENT_AXIS, axis_id)] = (ratio*axis_max - ratio*axis_min) / 65536.;
        m_axis_offset[make_pair(JS_EVENT_AXIS, axis_id)] = (ratio*axis_max + ratio*axis_min) * 0.5;
      }

      if(increment)
        m_increment_variable[value] = 0.;

      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }
  
  m_file_device = open(m_device_name.c_str(), O_RDONLY);
  if(m_file_device == -1)
    MOOSFail("Cannot open the joystick device '%s'.\n", m_device_name.c_str());
  
  m_deviceReadThread.Start();

  registerVariables();  
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void Joystick::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register("FOOBAR", 0);
  Register("TESTVAR", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

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
    m_msgs << "============================================ \n";
    m_msgs << "uJoystick status :                           \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "thread Quit Request | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << m_deviceReadThread.IsQuitRequested() << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();

  return(true);
}

//------------------------------------------------------------
// Procedure: deviceReadLoop()

bool Joystick::deviceReadLoop()
{
  while(!m_deviceReadThread.IsQuitRequested())
  {
    js_event e;
    fd_set fdset;
    timeval timeout = {0, 100000}; // sec, usec
    FD_ZERO(&fdset);
    FD_SET(m_file_device, &fdset);

    if(select(m_file_device + 1, &fdset, 0, 0, &timeout) > 0)
    {
      read(m_file_device, &e, sizeof(js_event));
      // do not differentiate between synthetic int events and real events
      e.type &= ~JS_EVENT_INIT;

      map<int,string>::iterator it;
      double value = m_axis_gain[make_pair(e.type,e.number)] * e.value + m_axis_offset[make_pair(e.type,e.number)];

      switch(e.type)
      {
        case JS_EVENT_BUTTON:
          it = m_button_variables.find(e.number);
          if(it != m_button_variables.end())
          {
            if(m_increment_variable.find(it->second) != m_increment_variable.end())
              m_increment_variable[it->second] = value;

            else
            {
              m_variables[it->second] = value;
              Notify(it->second, value);
            }
          }
          break;

        case JS_EVENT_AXIS:
          it = m_axis_variables.find(e.number);
          if(it != m_axis_variables.end())
          {
            if(m_increment_variable.find(it->second) != m_increment_variable.end())
              m_increment_variable[it->second] = value;
            else
            {
              m_variables[it->second] = value;
              Notify(it->second, value);
            }
          }
          break;

        default:
          break;
      }
    }
  }

  return true;
}

//------------------------------------------------------------
// Procedure: read_loop_thread_function()

bool Joystick::read_loop_thread_function(void* ptr_this)
{
  return ((Joystick*)ptr_this)->deviceReadLoop();
}