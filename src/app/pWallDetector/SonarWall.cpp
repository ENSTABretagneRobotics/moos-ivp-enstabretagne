/************************************************************/
/*    FILE: Nuc.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include "SonarWall.h"
#include <math.h>
#include <iterator>
#include <string>
#include <iostream>
#include <stdio.h>

using namespace std;

SonarWall::SonarWall(){
  // INITIALIZE VARIABLES
    m_mean_r       = 3;
    m_mean_theta   = 3;
    m_min_r        = 3;
    
    m_sonar_range  = 100.0;
    m_sonar_gain   = 105;
    
    m_new_scanline = false;
    
    m_threshold    = 80;
    m_search_zone  = 20;

    m_vertical_scan = false;    
}

void SonarWall::newSonarData(vector<double> new_scanline, double new_bearing, double imu_yaw){
	m_new_scanline = true;
	m_scanline_tab.push_back(new_scanline);

	if(m_vertical_scan == true){
      m_bearing_tab.push_back(new_bearing);
    }
    else{
      m_bearing_tab.push_back(imu_yaw + new_bearing);  
    }	

	// Erase the first row of the buffer scanline_tab
    if(m_scanline_tab.size()>2*m_mean_theta+1){
      m_scanline_tab.erase (m_scanline_tab.begin());
      m_bearing_tab.erase(m_bearing_tab.begin());
    }
}

bool SonarWall::compute(){
	if(m_new_scanline){
		
		m_new_scanline = false;

	   if(m_scanline_tab.size()==2*m_mean_theta+1){
	   		
	      vector<double> scanline_filtered;

	      for(int l=max(m_min_r, m_mean_r); l<(int)(m_scanline_tab[m_mean_theta].size())-m_mean_r; l++){
	        vector<double> tmp;
	        for(int r = l-m_mean_r; r<l+m_mean_r; r++){
	          for(int theta=0; theta<m_scanline_tab.size(); theta++){            
	            tmp.push_back(m_scanline_tab[theta][r]);
	          }
	        }
	        double value_min; int indice_min;
	        findMin(tmp, value_min, indice_min, 0, scanline_filtered.size());
	        scanline_filtered.push_back(value_min);
	      }
	      // Find the max of the scanline_filtered
	      int indice_filtered;
	      findMax(scanline_filtered, m_max_filtered, indice_filtered, 0, scanline_filtered.size());
	      cout << "COUT4" << '\n';
	      if(m_max_filtered > m_threshold){
	        // Find the max in the scanline near the maximum of the scanline_filtered
	        double max; int indice;
	        int search_zone = round(m_search_zone*m_scanline_tab[0].size());
	        findMax(m_scanline_tab[m_mean_theta], max, indice, indice_filtered-search_zone, indice_filtered+search_zone);

	        m_distance = (indice)* m_sonar_range/m_scanline_tab[0].size();
			m_bearing = m_bearing_tab[m_mean_r];

	        return true;
	      }
	  }
	}
	return false;
}

void SonarWall::reset(){
	m_scanline_tab.clear();
	m_bearing_tab.clear();
}

void SonarWall::findMax(vector<double> list, double &max, int &indice, int begin, int end){
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

void SonarWall::findMin(vector<double> list, double &min, int &indice, int begin, int end){

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
}