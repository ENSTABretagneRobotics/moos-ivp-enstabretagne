/************************************************************/
/*    FILE: Nuc.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef SonarWall_HEADER
#define SonarWall_HEADER

#include <vector>

class SonarWall
{
  public:
    SonarWall();
    ~SonarWall() {};

    double m_bearing;
    double m_distance;
    
    double m_threshold;

    bool m_vertical_scan;
  public:
    void newSonarData(std::vector<double> new_scanline, double new_bearing, double imu_yaw);
    bool compute();
    void reset();

  public:
	// FILTER CONFIGURATION
    int m_mean_r, m_mean_theta;
    int m_min_r;
    int m_search_zone;

    // SONAR CONFIGURATION
    double m_sonar_range, m_sonar_gain;

    // Data Analysis
    double m_max_filtered;

	// State variables
    std::vector< std::vector<double> > m_scanline_tab;
    std::vector<double> m_bearing_tab;
    double m_new_scanline;
    
  private:
    void findMin(std::vector<double> list, double &min, int &indice, int begin, int end);
    void findMax(std::vector<double> list, double &max, int &indice, int begin, int end);

};

#endif 