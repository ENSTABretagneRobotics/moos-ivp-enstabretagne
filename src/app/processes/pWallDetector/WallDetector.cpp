/************************************************************/
/*    FILE: WallDetector.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "math.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "WallDetector.h"

using namespace std;

//---------------------------------------------------------
// Constructor

WallDetector::WallDetector()
{
  m_imu_yaw = 0.0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool WallDetector::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "IMU_YAW"){
      m_imu_yaw = msg.GetDouble();
    }
    else if(msg.GetKey() == "SONAR_RAW_DATA_MICRON"){

      double new_bearing;
      vector<double> new_scanline;

      MOOSValFromString(new_bearing, msg.GetString(), "bearing", true);
      int nRows, nCols;
      MOOSVectorFromString(msg.GetString(), new_scanline, nRows, nCols);

      sonar_micron.newSonarData(new_scanline, new_bearing, m_imu_yaw);
    }
    else if(msg.GetKey() == "SONAR_RAW_DATA_MINIKING"){

      double new_bearing;
      vector<double> new_scanline;

      MOOSValFromString(new_bearing, msg.GetString(), "bearing", true);
      int nRows, nCols;
      MOOSVectorFromString(msg.GetString(), new_scanline, nRows, nCols);

      sonar_miniking.newSonarData(new_scanline, new_bearing, m_imu_yaw);
    }
    else if(msg.GetKey() == "SONAR_RAW_DATA"){

      double new_bearing;
      vector<double> new_scanline;

      MOOSValFromString(new_bearing, msg.GetString(), "bearing", true);
      int nRows, nCols;
      MOOSVectorFromString(msg.GetString(), new_scanline, nRows, nCols);

      sonar.newSonarData(new_scanline, new_bearing, m_imu_yaw);
    }

    // PARAMETERS
    else if(key == "WALL_THRESHOLD_MINIKING"){
      sonar_miniking.m_threshold= msg.GetDouble();
    }
    else if(key == "WALL_THRESHOLD_MICRON"){
      sonar_micron.m_threshold = msg.GetDouble();
    }
    else if(key == "WALL_THRESHOLD"){
      sonar_micron.m_threshold = msg.GetDouble();
    }

    else if ( key == "MICRON_PARAMS" && msg.IsString()){
      // Mise à jour des paramètres du sonar
      // Le message est de la forme "Range=25,Gain=45,Continuous=true"
      double sonar_range, sonar_gain;
      MOOSValFromString(sonar_range, msg.GetString(), "Range", true);
      MOOSValFromString(sonar_gain, msg.GetString(), "Gain", true);
      // Reset the scanline_tab to avoid size issues
      sonar_micron.reset();
      sonar_micron.m_sonar_range = sonar_range;
      sonar_micron.m_sonar_gain = sonar_gain;
    }
    else if ( key == "MINIKING_PARAMS" && msg.IsString()){
      // Mise à jour des paramètres du sonar
      // Le message est de la forme "Range=25,Gain=45,Continuous=true"
      double sonar_range, sonar_gain;
      MOOSValFromString(sonar_range, msg.GetString(), "Range", true);
      MOOSValFromString(sonar_gain, msg.GetString(), "Gain", true);
      // Reset the scanline_tab to avoid size issues
      sonar_micron.reset();
      sonar_micron.m_sonar_range = sonar_range;
      sonar_micron.m_sonar_gain = sonar_gain;
    }
    else if ( key == "SONAR_PARAMS" && msg.IsString()){
      // Mise à jour des paramètres du sonar
      // Le message est de la forme "Range=25,Gain=45,Continuous=true"
      double sonar_range, sonar_gain;
      MOOSValFromString(sonar_range, msg.GetString(), "Range", true);
      MOOSValFromString(sonar_gain, msg.GetString(), "Gain", true);
      // Reset the scanline_tab to avoid size issues
      sonar.reset();
      sonar.m_sonar_range = sonar_range;
      sonar.m_sonar_gain = sonar_gain;
    }

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool WallDetector::OnConnectToServer()
{
  RegisterVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool WallDetector::Iterate()
{
  AppCastingMOOSApp::Iterate();

  if(sonar_micron.compute()){
    stringstream ss;
    ss << "bearing=" << sonar_micron.m_bearing << ",";
    ss << "distance=" << sonar_micron.m_distance << ",";
    ss << "vertical=" << sonar_micron.m_vertical_scan;
    Notify("WALL_DETECTOR_MICRON", ss.str());
  }

  if(sonar_miniking.compute()){
    stringstream ss;
    ss << "bearing=" << sonar_miniking.m_bearing << ",";
    ss << "distance=" << sonar_miniking.m_distance << ",";
    ss << "vertical=" << sonar_miniking.m_vertical_scan;
    Notify("WALL_DETECTOR_MINIKING", ss.str());
  }

  if(sonar.compute()){
    stringstream ss;
    ss << "bearing=" << sonar.m_bearing << ",";
    ss << "distance=" << sonar.m_distance << ",";
    ss << "vertical=" << sonar.m_vertical_scan;
    Notify("WALL_DETECTOR", ss.str());
  }

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool WallDetector::OnStartUp()
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

    if(param == "SONAR_RANGE_MINIKING"){
      sonar_miniking.m_sonar_range = atof(value.c_str()); handled = true;
    }
    else if(param == "SONAR_GAIN_MINIKING"){
      sonar_miniking.m_sonar_gain = atof(value.c_str()); handled = true;
    }
    else if(param == "MEAN_R_MINIKING"){
      sonar_miniking.m_mean_r = atoi(value.c_str()); handled = true;
    }
    else if(param == "MEAN_THETA_MINIKING"){
      sonar_miniking.m_mean_theta = atoi(value.c_str()); handled = true;
    }
    else if(param == "MIN_R_MINIKING"){
      sonar_miniking.m_min_r = atoi(value.c_str()); handled = true;
    }
    else if(param == "WALL_THRESHOLD_MINIKING"){
      sonar_miniking.m_threshold = atof(value.c_str()); handled = true;
    }
    else if(param == "SEARCH_ZONE_MINIKING"){
      sonar_miniking.m_search_zone = atoi(value.c_str()); handled = true;
    }
    else if(param == "VERTICAL_SCAN_MINIKING"){
      if(toupper(value) == "TRUE"){
       sonar_miniking.m_vertical_scan = true; handled = true;  
      }
      else if(toupper(value) == "FALSE"){
        sonar_miniking.m_vertical_scan = false; handled = true;
      }
      else{
        handled = false;
      }
    }

    if(param == "SONAR_RANGE_MICRON"){
      sonar_micron.m_sonar_range = atof(value.c_str()); handled = true;
    }
    else if(param == "SONAR_GAIN_MICRON"){
      sonar_micron.m_sonar_gain = atof(value.c_str()); handled = true;
    }
    else if(param == "MEAN_R_MICRON"){
      sonar_micron.m_mean_r = atoi(value.c_str()); handled = true;
    }
    else if(param == "MEAN_THETA_MICRON"){
      sonar_micron.m_mean_theta = atoi(value.c_str()); handled = true;
    }
    else if(param == "MIN_R_MICRON"){
      sonar_micron.m_min_r = atoi(value.c_str()); handled = true;
    }
    else if(param == "WALL_THRESHOLD_MICRON"){
      sonar_micron.m_threshold = atof(value.c_str()); handled = true;
    }
    else if(param == "SEARCH_ZONE_MICRON"){
      sonar_micron.m_search_zone = atoi(value.c_str()); handled = true;
    }
    else if(param == "VERTICAL_SCAN_MICRON"){
      if(toupper(value) == "TRUE"){
        sonar_micron.m_vertical_scan = true; handled = true;  
      }
      else if(toupper(value) == "FALSE"){
        sonar_micron.m_vertical_scan = false; handled = true;
      }
      else{
        handled = false;
      }
    }

    if(param == "SONAR_RANGE"){
      sonar.m_sonar_range = atof(value.c_str()); handled = true;
    }
    else if(param == "SONAR_GAIN"){
      sonar.m_sonar_gain = atof(value.c_str()); handled = true;
    }
    else if(param == "MEAN_R"){
      sonar.m_mean_r = atoi(value.c_str()); handled = true;
      cout << sonar.m_mean_r<< '\n';
    }
    else if(param == "MEAN_THETA"){
      sonar.m_mean_theta = atoi(value.c_str()); handled = true;
    }
    else if(param == "MIN_R"){
      sonar.m_min_r = atoi(value.c_str()); handled = true;
    }
    else if(param == "WALL_THRESHOLD"){
      sonar.m_threshold = atof(value.c_str()); handled = true;
    }
    else if(param == "SEARCH_ZONE"){
      sonar.m_search_zone = atoi(value.c_str()); handled = true;
    }
    else if(param == "VERTICAL_SCAN"){
      if(toupper(value) == "TRUE"){
        sonar.m_vertical_scan = true; handled = true;  
      }
      else if(toupper(value) == "FALSE"){
        sonar.m_vertical_scan = false; handled = true;
      }
      else{
        handled = false;
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

void WallDetector::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("IMU_YAW", 0);
  
  Register("SONAR_RAW_DATA", 0);
  Register("WALL_THRESHOLD", 0);
  Register("SONAR_PARAMS", 0);

  Register("SONAR_RAW_DATA_MINIKING", 0);
  Register("WALL_THRESHOLD_MINIKING", 0);
  Register("MINIKING_PARAMS", 0);

  Register("SONAR_RAW_DATA_MICRON", 0);
  Register("WALL_THRESHOLD_MICRON", 0);
  Register("MICRON_PARAMS", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool WallDetector::buildReport() 
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

  m_msgs << "MICRON SONAR" << '\n';
  ACTable actab(5);
  actab << "Nb Data | Size Data | Max filtered | Wall Threshold | Max Range";
  actab.addHeaderLines();
  if(sonar_micron.m_scanline_tab.size()==0)
    actab << (int)sonar_micron.m_scanline_tab.size() 
          << "x"
          << sonar_micron.m_max_filtered 
          << sonar_micron.m_threshold 
          << sonar_micron.m_sonar_range;
  else
    actab << (int)sonar_micron.m_scanline_tab.size() 
          << (int)sonar_micron.m_scanline_tab[0].size() 
          << sonar_micron.m_max_filtered 
          << sonar_micron.m_threshold 
          << sonar_micron.m_sonar_range;
  m_msgs << actab.getFormattedString();


  m_msgs << '\n' << "MINIKING SONAR" << '\n';
  ACTable actab2(5);
  actab2 << "Nb Data | Size Data | Max filtered | Wall Threshold | Max Range";
  actab2.addHeaderLines();
  if(sonar_micron.m_scanline_tab.size()==0)
    actab2 << (int)sonar_micron.m_scanline_tab.size() 
          << "x" 
          << sonar_micron.m_max_filtered 
          << sonar_micron.m_threshold 
          << sonar_micron.m_sonar_range;
  else
    actab2 << (int)sonar_micron.m_scanline_tab.size() 
          << (int)sonar_micron.m_scanline_tab[0].size() 
          << sonar_micron.m_max_filtered 
          << sonar_micron.m_threshold 
          << sonar_micron.m_sonar_range;
  m_msgs << actab2.getFormattedString();

  m_msgs << '\n' << "SONAR" << '\n';
  ACTable actab3(5);
  actab3 << "Nb Data | Size Data | Max filtered | Wall Threshold | Max Range";
  actab3.addHeaderLines();
  if(sonar.m_scanline_tab.size()==0)
    actab3 << (int)sonar.m_scanline_tab.size() 
          << "x" 
          << sonar.m_max_filtered 
          << sonar.m_threshold 
          << sonar.m_sonar_range;
  else
    actab3 << (int)sonar.m_scanline_tab.size() 
          << (int)sonar.m_scanline_tab[0].size() 
          << sonar.m_max_filtered 
          << sonar.m_threshold 
          << sonar.m_sonar_range;
  m_msgs << actab3.getFormattedString();

  return true;
}