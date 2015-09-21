/************************************************************/
/*    FILE: SensorViewer.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH:
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "SensorViewer.h"

#define LARGEUR_IMAGE_CAMERA			360
#define HAUTEUR_IMAGE_CAMERA			288

using namespace std;

/**
 * \fn
 * \brief Constructeur de l'application MOOS
 */

SensorViewer::SensorViewer()
{
	m_size_scanline_miniking = 200;
	m_size_scanline_micron = 200;

	m_img_camera_front.create(HAUTEUR_IMAGE_CAMERA,LARGEUR_IMAGE_CAMERA,CV_8UC3);
	m_img_camera_bottom.create(HAUTEUR_IMAGE_CAMERA,LARGEUR_IMAGE_CAMERA,CV_8UC3);
	m_img_sonar_micron.create(2*m_size_scanline_micron,2*m_size_scanline_micron,CV_8UC3);
	m_img_sonar_miniking.create(2*m_size_scanline_miniking,2*m_size_scanline_miniking,CV_8UC3);


	m_old_bearing_miniking = 0.0;
	m_old_bearing_micron = 0.0;

	m_new_bearing_miniking = 0.0;
	m_new_bearing_micron = 0.0;
}

SensorViewer::~SensorViewer()
{
}

bool SensorViewer::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
	MOOSMSG_LIST::iterator p;

	for(p = NewMail.begin() ; p != NewMail.end() ; p++)
	{
		CMOOSMsg &msg = *p;

		#if 0 // Keep these around just for template
		string key   = msg.GetKey();
		string comm  = msg.GetCommunity();
		double dval  = msg.GetDouble();
		string sval  = msg.GetString();
		string msrc  = msg.GetSource();
		double mtime = msg.GetTime();
		bool   mdbl  = msg.IsDouble();
		bool   mstr  = msg.IsString();
		#endif

		//cout << msg.GetKey() << en
		double pool_angle = MOOSDeg2Rad(0.0);

		// **************** SENSORS DATA ************************
		// 
		if( msg.GetKey() == "CAMERA_SIDE") {
		  	memcpy(m_img_camera_bottom.data, msg.GetBinaryData(), m_img_camera_bottom.rows*m_img_camera_bottom.step);
		}
		else if( msg.GetKey() == "CAMERA_BOTTOM") {
		  	memcpy(m_img_camera_front.data, msg.GetBinaryData(), m_img_camera_front.rows*m_img_camera_front.step);
		}
		else if(msg.GetKey() == "SONAR_SCANLINE_MICRON"){
			int nRows, nCols;
      		MOOSVectorFromString(msg.GetString(), m_new_scanline_micron, nRows, nCols);
      		if(m_new_scanline_micron.size() != m_size_scanline_micron){
      			m_img_sonar_micron.create(2*m_new_scanline_micron.size(),2*m_new_scanline_micron.size(),CV_8UC3);
      		}
      		m_size_scanline_micron = m_new_scanline_micron.size();
      		m_new_data_micron = true;
		}
		else if(msg.GetKey() == "SONAR_SCANLINE_MINIKING"){
			int nRows, nCols;
      		MOOSVectorFromString(msg.GetString(), m_new_scanline_miniking, nRows, nCols);
      		if(m_new_scanline_miniking.size() != m_size_scanline_miniking){
      			m_img_sonar_miniking.create(2*m_new_scanline_miniking.size(),2*m_new_scanline_miniking.size(),CV_8UC3);
      		}
      		m_size_scanline_miniking = m_new_scanline_miniking.size();
      		m_new_data_miniking = true;
		}
		else if(msg.GetKey() == "SONAR_BEARING_MICRON"){
			m_new_bearing_micron = msg.GetDouble();
		}
		else if(msg.GetKey() == "SONAR_BEARING_MINIKING"){
			m_new_bearing_miniking = msg.GetDouble();
		}

		// **************** ANALYSIS DATA ************************
		// 
	    // if( msg.GetKey() == "WALL_DETECTOR"){
	    //   MOOSValFromString(m_bearing, msg.GetString(), "bearing");
	    //   MOOSValFromString(m_distance, msg.GetString(), "distance");
	    // }

		// **************** CONFIG DATA ************************
		// 
		// if( msg.GetKey() == "SONAR_VIEW_SCALE") {
		//   m_dSonarScaleFactor = msg.GetDouble();
		// }
	}
	return(true);
}

bool SensorViewer::OnConnectToServer()
{
	RegisterVariables();
	return(true);
}

bool SensorViewer::Iterate()
{
	AppCastingMOOSApp::Iterate();

	// MICRON
	if(m_new_data_micron){
		AddSector(m_img_sonar_micron, m_new_scanline_micron, m_new_bearing_micron, m_old_bearing_micron);
		m_new_data_micron = false;
	}
	if(m_new_data_miniking){
		AddSector(m_img_sonar_miniking, m_new_scanline_miniking, m_new_bearing_miniking, m_old_bearing_micron);
		m_new_data_miniking = false;
	}

	cv::imshow("SONAR MINIKING", m_img_sonar_miniking);
	cv::imshow("SONAR MICRON", m_img_sonar_micron);
	cv::imshow("CAMERA FRONT", m_img_camera_front);
	cv::imshow("CAMERA BOTTOM", m_img_camera_bottom);

	cv::waitKey(1);
	AppCastingMOOSApp::PostReport();
	return(true);
}

bool SensorViewer::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
	setlocale(LC_ALL, "C");
	list<string> sParams;
	m_MissionReader.EnableVerbatimQuoting(false);
	if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
	{
		list<string>::iterator p;
		for(p = sParams.begin() ; p != sParams.end() ; p++)
		{
			string original_line = *p;
			string param = stripBlankEnds(toupper(biteString(*p, '=')));
			string value = stripBlankEnds(*p);

			if(param == "FOO")
			{
				//handled
			}

			else if(param == "BAR")
			{
				//handled
			}
		}
	}

	RegisterVariables();

	cv::namedWindow("CAMERA FRONT");
	cv::namedWindow("CAMERA BOTTOM");
	cv::namedWindow("SONAR MINIKING");
	cv::namedWindow("SONAR MICRON");

	return(true);
}

void SensorViewer::RegisterVariables()
{
	AppCastingMOOSApp::RegisterVariables();

	// CAMERA DATA
	Register("CAMERA_SIDE", 0);
	Register("CAMERA_BOTTOM", 0);

	// SONAR DATA
	Register("SONAR_SCANLINE_MICRON", 0);
	Register("SONAR_SCANLINE_MINIKING", 0);
	Register("SONAR_BEARING_MICRON", 0);
	Register("SONAR_BEARING_MINIKING", 0);

}
bool SensorViewer::buildReport()
{
  #if 0 // Keep these around just for template
    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif

  return(true);
}

void SensorViewer::AddSector(cv::Mat &img_sonar, vector<double> scanline, double bearing, double &bearing_previous){
	for(int r=1; r<scanline.size(); r++){
		double angle_diff = 2*atan(tan(abs(bearing - bearing_previous)/2));
		double theta_begin = min(bearing, bearing_previous);

		for(double theta=theta_begin; theta<theta_begin + angle_diff; theta+=1/r){
			int x = round(scanline.size()/2.0 + r*cos(theta));
			int y = round(scanline.size()/2.0 + r*sin(theta));
			img_sonar.at<unsigned char>(x,y) = 255 - (unsigned char)scanline[r];
		}
		bearing_previous = bearing;
	}
}