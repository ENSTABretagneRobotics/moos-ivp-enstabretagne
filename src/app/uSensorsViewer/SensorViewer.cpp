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
using namespace cv;

/**
 * \fn
 * \brief Constructeur de l'application MOOS
 */

SensorViewer::SensorViewer()
{
	m_size_scanline_miniking = 200;
	m_size_scanline_micron = 200;
	m_size_scanline = 200;

	m_img_camera_side.create(HAUTEUR_IMAGE_CAMERA,LARGEUR_IMAGE_CAMERA,CV_8UC(3));
	m_img_camera_bottom.create(HAUTEUR_IMAGE_CAMERA,LARGEUR_IMAGE_CAMERA,CV_8UC(3));
	m_img_sonar_micron.create(2*m_size_scanline_micron,2*m_size_scanline_micron,CV_8UC1);
	m_img_sonar_miniking.create(2*m_size_scanline_miniking,2*m_size_scanline_miniking,CV_8UC1);
	m_img_sonar.create(2*m_size_scanline, 2*m_size_scanline, CV_8UC1);


	m_old_bearing_miniking = 0.0;
	m_old_bearing_micron = 0.0;
	m_old_bearing = 0.0;

	m_new_bearing_miniking = 0.0;
	m_new_bearing_micron = 0.0;
	m_new_bearing = 0.0;

	m_new_data = false;
	m_new_data_micron = false;
	m_new_data_miniking = false;

	m_sonar_contrast = 20.0;
	m_sonar_contrast_micron = 20.0;
	m_sonar_contrast_miniking = 20.0;
}

SensorViewer::~SensorViewer()
{
	destroyAllWindows();
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
		  	memcpy(m_img_camera_side.data, msg.GetBinaryData(), m_img_camera_side.rows*m_img_camera_side.step);
		}
		else if(msg.GetKey() == "SONAR_RAW_DATA_MICRON"){
			int nRows, nCols;
			std::string msg_scanline;
			MOOSValFromString(msg_scanline, msg.GetString(), "scanline", true);
			m_new_scanline_micron.clear();
      		MOOSVectorFromString(msg_scanline, m_new_scanline_micron, nRows, nCols);
      		if(m_new_scanline_micron.size() != m_size_scanline_micron){
      			m_img_sonar_micron.create(2*m_new_scanline_micron.size(),2*m_new_scanline_micron.size(),CV_8UC(1));
      			m_img_sonar_micron.setTo(Scalar(255));
      		}
      		m_size_scanline_micron = m_new_scanline_micron.size();
      		m_new_data_micron = true;
		}
		else if(msg.GetKey() == "SONAR_RAW_DATA_MINIKING"){
			int nRows, nCols;
			std::string msg_scanline;
			MOOSValFromString(msg_scanline, msg.GetString(), "scanline", true);
			m_new_scanline_miniking.clear();
      		MOOSVectorFromString(msg.GetString(), m_new_scanline_miniking, nRows, nCols);
      		if(m_new_scanline_miniking.size() != m_size_scanline_miniking){
      			m_img_sonar_miniking.create(2*m_new_scanline_miniking.size(),2*m_new_scanline_miniking.size(),CV_8UC(1));
      			m_img_sonar_miniking.setTo(Scalar(255));
      		}
      		m_size_scanline_miniking = m_new_scanline_miniking.size();
      		m_new_data_miniking = true;
		}
		else if(msg.GetKey() == "SONAR_RAW_DATA"){
			double bearing_tmp;
			MOOSValFromString(bearing_tmp, msg.GetString(), "bearing", true);
			m_new_bearing = bearing_tmp*M_PI/180.0;
			
			int nRows, nCols;
			std::string msg_scanline;
			MOOSValFromString(msg_scanline, msg.GetString(), "scanline", true);
			m_new_scanline.clear();
      		MOOSVectorFromString(msg.GetString(), m_new_scanline, nRows, nCols);
      		if(m_new_scanline.size() != m_size_scanline){
      			m_img_sonar.create(2*m_new_scanline.size(),2*m_new_scanline.size(),CV_8UC(1));
      			m_img_sonar.setTo(Scalar(255));
      		}
      		m_size_scanline = m_new_scanline.size();
      		m_new_data = true;
		}

		// **************** ANALYSIS DATA ************************
		// 
	    // if( msg.GetKey() == "WALL_DETECTOR"){
	    //   MOOSValFromString(m_bearing, msg.GetString(), "bearing");
	    //   MOOSValFromString(m_distance, msg.GetString(), "distance");
	    // }

		// **************** CONFIG DATA ************************
		
		else if(msg.GetKey() == "SONAR_MINIKING_CONTRAST")
			m_sonar_contrast_miniking = msg.GetDouble();
		else if(msg.GetKey() == "SONAR_MICRON_CONTRAST")
			m_sonar_contrast_micron = msg.GetDouble();
		else if(msg.GetKey() == "SONAR_CONTRAST")
			m_sonar_contrast = msg.GetDouble();
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
		AddSector(m_img_sonar_micron, m_new_scanline_micron, m_new_bearing_micron, m_old_bearing_micron, m_sonar_contrast_micron);
		m_new_data_micron = false;
	}
	if(m_new_data_miniking){
		AddSector(m_img_sonar_miniking, m_new_scanline_miniking, m_new_bearing_miniking, m_old_bearing_micron, m_sonar_contrast_miniking);
		m_new_data_miniking = false;
	}
	if(m_new_data){
		AddSector(m_img_sonar, m_new_scanline, m_new_bearing, m_old_bearing, m_sonar_contrast);
		m_new_data = false;
	}

	imshow("SONAR MINIKING", m_img_sonar_miniking);
	imshow("SONAR MICRON", m_img_sonar_micron);
	imshow("SONAR", m_img_sonar);
	
	imshow("CAMERA SIDE", m_img_camera_side);
	imshow("CAMERA BOTTOM", m_img_camera_bottom);

	waitKey(1);
	AppCastingMOOSApp::PostReport();
	return(true);
}

bool SensorViewer::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    STRING_LIST::iterator p;
    for (p = sParams.begin(); p != sParams.end(); p++) {
        string orig = *p;
        string line = *p;
        string param = toupper(biteStringX(line, '='));
        string value = line;

        bool handled = false;
        if (param == "SONAR_CONTRAST") {
            m_sonar_contrast = atof(value.c_str());
            handled = true;
        }
        else if (param == "SONAR_CONTRAST_MINIKING") {
            m_sonar_contrast_miniking = atof(value.c_str());
            handled = true;
        }
        else if (param == "SONAR_CONTRAST_MICRON") {
            m_sonar_contrast_micron = atof(value.c_str());
            handled = true;
        }

        if (!handled)
            reportUnhandledConfigWarning(orig);
    }

	registerVariables();

	namedWindow("CAMERA SIDE", WINDOW_NORMAL);
	namedWindow("CAMERA BOTTOM", WINDOW_NORMAL);
	namedWindow("SONAR MINIKING", WINDOW_NORMAL);
	namedWindow("SONAR MICRON", WINDOW_NORMAL);
	namedWindow("SONAR", WINDOW_NORMAL);

	return(true);
}

void SensorViewer::registerVariables()
{
	AppCastingMOOSApp::RegisterVariables();

	// CAMERA DATA
	Register("CAMERA_SIDE", 0);
	Register("CAMERA_BOTTOM", 0);

	// SONAR DATA
	Register("SONAR_RAW_DATA", 0);
	Register("SONAR_RAW_DATA_MICRON", 0);
	Register("SONAR_RAW_DATA_MINIKING", 0);

	Register("SONAR_MINIKING_CONTRAST", 0);
	Register("SONAR_MICRON_CONTRAST", 0);
	Register("SONAR_CONTRAST", 0);

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

void SensorViewer::AddSector(Mat &img_sonar, vector<double> scanline, double bearing, double &bearing_previous, double contrast){
	for(int r=1; r<scanline.size(); r++){
		double angle_diff = abs(2*atan(tan(abs(bearing - bearing_previous)/2)));
		double theta_begin = min(bearing, bearing_previous);

		for(double theta=theta_begin; theta<theta_begin + angle_diff; theta+=1.0/r){
			int x = floor(scanline.size() + r*cos(theta));
			int y = floor(scanline.size() + r*sin(theta));
			img_sonar.at<unsigned char>(y, x) = 255 - pow(10, scanline[r]/20.0)/pow(10, 150/20.0)*255*255*contrast;
			// cout << "SCANLINE SIZE = " << scanline.size() << " r = " << r << '\n';
			// cout << "ADD SECTOR (" << x << " " << y << ") = " << 255 - (unsigned char)scanline[r] << '\n';
		}
	}

	bearing_previous = bearing;
}