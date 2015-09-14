/************************************************************/
/*    FILE: WallDetector.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: 
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "WallDetector.h"

using namespace std;

//---------------------------------------------------------
// Constructor

WallDetector::WallDetector()
{
  // INITIALIZE VARIABLES
    mean_r = 3;
    mean_theta = 3;
    min_r = 10;

    sonar_range = 100.0;
    sonar_nbins = 100;
    sonar_gain = 105;

    imu_yaw = 0.0;
    new_scanline = false;

    threshold = 80;
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

    if(key == "FOO") 
      cout << "great!";
    else if(key == "SONAR_BEARING"){
      new_bearing = msg.GetDouble();
    }
    else if(key == "SONAR_SCANLINE"){
      new_scanline = true;
      int nRows, nCols;
      MOOSVectorFromString(msg.GetString(), new_scanline_data, nRows, nCols);
    }

    else if(key == "IMU_YAW"){
      imu_yaw = msg.GetDouble();
    }
    else if(key == "WALL_THRESHOLD"){
      threshold = msg.GetDouble();
    }

    // Mise à jour des paramètres du sonar
    else if ( key == "SONAR_PARAMS" && msg.IsString()){
      string msg_val = msg.GetString();
      // Le message est de la forme "Range=25,Gain=45,Continuous=true"
      double dVal=0.0; int iVal; bool bVal;
      if (MOOSValFromString(dVal, msg_val, "Range", true))
        sonar_range = dVal;
      if (MOOSValFromString(iVal, msg_val, "nBins", true))
        sonar_nbins = iVal;
      if (MOOSValFromString(dVal, msg_val, "Gain", true))
        sonar_gain = dVal;

      // Reset the scanline_tab to avoid size issues
      scanline_tab.clear();
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
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool WallDetector::Iterate()
{
  AppCastingMOOSApp::Iterate();

  // Do your thing here!
  if(new_scanline == true){
    new_scanline = false;

    // Push back new incoming scanline
    scanline_tab.push_back(new_scanline_data);
    // Convert bearing into radian + add imu_yaw
    bearing_tab.push_back(MOOS_ANGLE_WRAP(MOOSDeg2Rad(imu_yaw) + MOOSGrad2Rad(new_bearing/16.0)));

    // Erase the first row of the buffer scanline_tab
    if(scanline_tab.size()>2*mean_theta){
      scanline_tab.erase (scanline_tab.begin());
      bearing_tab.erase(bearing_tab.begin());
    }

    // Compute the filtered value of the scanline
    // min value in the neighborhood
    if(scanline_tab.size()==2*mean_theta){
      vector<double> scanline_filtered;

      for(int l=min_r; l<scanline_tab[0].size()-mean_r; l++){
        vector<double> tmp;
        for(int i = l-mean_r; i<l+mean_r+1; i++){
          for(int j=0; j<scanline_tab.size(); j++){
            tmp.push_back(scanline_tab[j][i]);
          }
        }
        scanline_filtered.push_back(*min_element(tmp.begin(), tmp.end()));
      }

      vector<double>::iterator max = max_element(scanline_filtered.begin(), scanline_filtered.end());

      if(*max > threshold){
        double dist = (std::distance(scanline_filtered.begin(), max) + min_r)* sonar_range/scanline_tab[0].size();
        stringstream ss;
        ss << "bearing=" << bearing_tab[mean_r] << ",";
        ss << "distance=" << dist << ",";
        Notify("WALL_DETECTOR", ss.str());
      }

    }
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

    if(param == "SONAR_RANGE"){
      sonar_range = atof(value.c_str());
      handled = true;
    }
    else if(param == "SONAR_NBINS"){
      sonar_nbins = atoi(value.c_str());
      handled = true;
    }
    else if(param == "SONAR_GAIN"){
      sonar_gain = atof(value.c_str());
      handled = true;
    }
    else if(param == "MEAN_R"){
      mean_r = atoi(value.c_str());
      handled = true;
    }
    else if(param == "MEAN_THETA"){
      mean_theta = atoi(value.c_str());
      handled = true;
    }
    else if(param == "MIN_R"){
      min_r = atoi(value.c_str());
      handled = true;
    }
    else if(param == "WALL_THRESHOLD"){
      threshold = atof(value.c_str());
      handled = true;
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
  Register("SONAR_PARAMS", 0);
  Register("WALL_THRESHOLD", 0);
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

  return true;
}

double WallDetector::MOOSGrad2Rad(double angle){
  return angle*M_PI/200.0;
}