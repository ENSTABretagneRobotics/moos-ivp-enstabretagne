/************************************************************/
/*    FILE: WallDetector.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
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
  // INITIALIZE VARIABLES
    m_mean_r       = 3;
    m_mean_theta   = 3;
    m_min_r        = 3;
    
    m_sonar_range  = 100.0;
    m_sonar_gain   = 105;
    
    m_imu_yaw      = 0.0;
    m_new_scanline = false;
    
    m_threshold    = 80;
    m_search_zone  = 20;
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

    if(key == "SONAR_BEARING"){
      m_new_bearing = msg.GetDouble();
    }
    else if(key == "SONAR_SCANLINE"){
      m_new_scanline = true;
      int nRows, nCols;
      MOOSVectorFromString(msg.GetString(), m_new_scanline_data, nRows, nCols);
      if(m_new_scanline_data.size()==0){
        reportRunWarning("ERROR Parsing SONAR_SCANLINE");
      }
    }
    else if(key == "IMU_YAW"){
      m_imu_yaw = msg.GetDouble();
    }

    // PARAMETERS
    else if(key == "WALL_THRESHOLD"){
      m_threshold = msg.GetDouble();
    }
    else if ( key == "SONAR_PARAMS" && msg.IsString()){
      // Mise à jour des paramètres du sonar
      // Le message est de la forme "Range=25,Gain=45,Continuous=true"
      MOOSValFromString(m_sonar_range, msg.GetString(), "Range", true);
      MOOSValFromString(m_sonar_gain, msg.GetString(), "Gain", true);
      // Reset the scanline_tab to avoid size issues
      m_scanline_tab.clear();
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
  if(m_new_scanline == true){
    m_new_scanline = false;

    // Push back new incoming scanline
    m_scanline_tab.push_back(m_new_scanline_data);
    // Convert bearing into radian + add imu_yaw
    if(m_vertical_scan == true){
      m_bearing_tab.push_back(MOOSGrad2Rad(m_new_bearing/16.0));
    }
    else{
      m_bearing_tab.push_back(MOOS_ANGLE_WRAP(MOOSDeg2Rad(m_imu_yaw) + MOOSGrad2Rad(m_new_bearing/16.0)));  
    }
    
    // Erase the first row of the buffer scanline_tab
    if(m_scanline_tab.size()>2*m_mean_theta+1){
      m_scanline_tab.erase (m_scanline_tab.begin());
      m_bearing_tab.erase(m_bearing_tab.begin());
    }

    // Compute the filtered value of the scanline
    // min value in the neighborhood
    if(m_scanline_tab.size()==2*m_mean_theta+1){

      vector<double> scanline_filtered;

      for(int l=max(m_min_r, m_mean_r); l<m_scanline_tab[m_mean_theta].size()-m_mean_r; l++){
        vector<double> tmp;
        for(int r = l-m_mean_r; r<l+m_mean_r; r++){
          for(int theta=0; theta<m_scanline_tab.size(); theta++){            
            tmp.push_back(m_scanline_tab[theta][r]);
          }
        }
        scanline_filtered.push_back(*min_element(tmp.begin(), tmp.end()));
      }

      // Find the max of the scanline_filtered
      int indice_filtered;
      findMax(scanline_filtered, m_max_filtered, indice_filtered, 0, scanline_filtered.size());

      if(m_max_filtered > m_threshold){
        // Find the max in the scanline near the maximum of the scanline_filtered
        double max; int indice;
        int search_zone = round(m_search_zone*m_scanline_tab[0].size());
        findMax(m_scanline_tab[m_mean_theta], max, indice, indice_filtered-search_zone, indice_filtered+search_zone);

        double dist = (indice)* m_sonar_range/m_scanline_tab[0].size();
        stringstream ss;
        ss << "bearing=" << m_bearing_tab[m_mean_r] << ",";
        ss << "distance=" << dist << ",";
        ss << "vertical=" << m_vertical_scan;
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
      m_sonar_range = atof(value.c_str());
      handled = true;
    }
    else if(param == "SONAR_GAIN"){
      m_sonar_gain = atof(value.c_str());
      handled = true;
    }
    else if(param == "MEAN_R"){
      m_mean_r = atoi(value.c_str());
      handled = true;
    }
    else if(param == "MEAN_THETA"){
      m_mean_theta = atoi(value.c_str());
      handled = true;
    }
    else if(param == "MIN_R"){
      m_min_r = atoi(value.c_str());
      handled = true;
    }
    else if(param == "WALL_THRESHOLD"){
      m_threshold = atof(value.c_str());
      handled = true;
    }
    else if(param == "SEARCH_ZONE"){
      m_search_zone = atoi(value.c_str());
      handled = true;
    }
    else if(param == "VERTICAL_SCAN"){
      if(toupper(value) == "TRUE"){
        m_vertical_scan = true;
        handled = true;  
      }
      else if(toupper(value) == "FALSE"){
        m_vertical_scan = false;
        handled = true;
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
  Register("SONAR_PARAMS", 0);

  Register("WALL_THRESHOLD", 0);
  Register("SONAR_BEARING", 0);
  Register("SONAR_SCANLINE", 0);
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
    m_msgs << "============================================ \n";
    m_msgs << "File: pWallDetector                          \n";
    m_msgs << "============================================ \n";

    ACTable actab(5);
    actab << "Nb Data | Size Data | Max filtered | Wall Threshold | Max Range";
    actab.addHeaderLines();
    if(m_scanline_tab.size()==0)
      actab << (int)m_scanline_tab.size() << "x" << m_max_filtered << m_threshold << m_sonar_range;
    else
      actab << (int)m_scanline_tab.size() << (int)m_scanline_tab[0].size() << m_max_filtered << m_threshold << m_sonar_range;
    m_msgs << actab.getFormattedString();

  return true;
}

double WallDetector::MOOSGrad2Rad(double angle){
  return angle*M_PI/200.0;
}

void WallDetector::findMax(vector<double> list, double &max, int &indice, int begin, int end){

  if(begin>= list.size())
    begin = list.size()-1;
  if(end > list.size())
    end = list.size();
  if(begin < 0)
    begin = 0;
  if(end < 0)
    end = 0;

  max = list[begin];
  indice = begin;

  for(int i=begin+1; i<end; i++){
    if(list[i]>max){
      max = list[i];
      indice = i;
    }
  }
}

void WallDetector::findMin(vector<double> list, double &min, int &indice, int begin, int end){

  if(begin>= list.size())
    begin = list.size()-1;
  if(end > list.size())
    end = list.size();
  if(begin < 0)
    begin = 0;
  if(end < 0)
    end = 0;

  min = list[begin];
  indice = begin;

  for(int i=begin+1; i<end; i++){
    if(list[i]<min){
      min = list[i];
      indice = i;
    }
  }

  cout << min;
}