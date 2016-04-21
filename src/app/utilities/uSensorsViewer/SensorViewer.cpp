/************************************************************/
/*    FILE: SensorViewer.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
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

SensorViewer::SensorViewer():
    m_camera_bottom_keyname("CAMERA_BOTTOM"), m_camera_side_keyname("CAMERA_SIDE"),
    m_sonar_micron_keyname("SONAR_RAW_DATA_MICRON"), m_sonar_micron_contrast_keyname("SONAR_MICRON_CONTRAST"),
    m_sonar_minking_keyname("SONAR_RAW_DATA_MINIKING"), m_sonar_minking_contrast_keyname("SONAR_MINIKING_CONTRAST"),
    m_sonar_raw_keyname("SONAR_RAW_DATA"), m_sonar_contrast_keyname("SONAR_CONTRAST"),
    m_wall_detector_minking_keyname("WALL_DETECTOR_MINIKING"), m_wall_detector_micron_keyname("WALL_DETECTOR_MICRON")

{
	m_nb_camera_bottom = 0;
	m_nb_camera_side = 0;

	m_size_scanline_miniking = 200;
	m_size_scanline_micron = 200;
	m_size_scanline = 200;

	m_img_camera_side.create(HAUTEUR_IMAGE_CAMERA,LARGEUR_IMAGE_CAMERA,CV_8UC(3));
	m_img_camera_bottom.create(HAUTEUR_IMAGE_CAMERA,LARGEUR_IMAGE_CAMERA,CV_8UC(3));

	m_img_sonar_micron.create(2*m_size_scanline_micron,2*m_size_scanline_micron,CV_8UC1);
	m_img_sonar_miniking.create(2*m_size_scanline_miniking,2*m_size_scanline_miniking,CV_8UC1);
	m_img_sonar.create(2*m_size_scanline, 2*m_size_scanline, CV_8UC1);

	m_img_micron.create(2*m_size_scanline_micron,2*m_size_scanline_micron,CV_8UC(3));

	m_img_wall_micron.create(500, 500, CV_8UC1);
	m_img_wall_miniking.create(500, 500, CV_8UC1);

	circle(m_img_wall_micron, Point(250, 250), 5, 255);
	circle(m_img_wall_miniking, Point(250, 250), 5, 255);

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

        // **************** CAMERA DATA ************************
        if( msg.GetKey() == m_camera_bottom_keyname) {
			m_nb_camera_bottom ++;
		  	memcpy(m_img_camera_bottom.data, msg.GetBinaryData(), m_img_camera_bottom.rows*m_img_camera_bottom.step);
		}
        else if( msg.GetKey() == m_camera_side_keyname) {
		  	m_nb_camera_side ++;
		  	memcpy(m_img_camera_side.data, msg.GetBinaryData(), m_img_camera_side.rows*m_img_camera_side.step);
		}

        // **************** SONAR DATA ************************
        else if(msg.GetKey() == m_sonar_micron_keyname){
			MOOSValFromString(m_new_bearing_micron, msg.GetString(), "bearing", true);

			int nRows, nCols;
			m_new_scanline_micron.clear();
      		MOOSVectorFromString(msg.GetString(), m_new_scanline_micron, nRows, nCols);
      		if(m_new_scanline_micron.size() != m_size_scanline_micron){
      			m_img_sonar_micron.create(2*m_new_scanline_micron.size(),2*m_new_scanline_micron.size(),CV_8UC(1));
      			m_img_micron.create(2*m_new_scanline_micron.size(),2*m_new_scanline_micron.size(),CV_8UC(3));
      			m_img_sonar_micron.setTo(Scalar(255));
      			m_img_micron.setTo(Scalar(255, 255, 255));
      		}
      		m_size_scanline_micron = m_new_scanline_micron.size();
      		m_new_data_micron = true;
		}
        else if(msg.GetKey() == m_sonar_minking_keyname){
			MOOSValFromString(m_new_bearing_miniking, msg.GetString(), "bearing", true);

			int nRows, nCols;
			m_new_scanline_miniking.clear();
      		MOOSVectorFromString(msg.GetString(), m_new_scanline_miniking, nRows, nCols);
      		if(m_new_scanline_miniking.size() != m_size_scanline_miniking){
      			m_img_sonar_miniking.create(2*m_new_scanline_miniking.size(),2*m_new_scanline_miniking.size(),CV_8UC(1));
      			m_img_sonar_miniking.setTo(Scalar(255));
      		}
      		m_size_scanline_miniking = m_new_scanline_miniking.size();
      		m_new_data_miniking = true;
		}
        else if(msg.GetKey() == m_sonar_raw_keyname){
			MOOSValFromString(m_new_bearing, msg.GetString(), "bearing", true);
			
			int nRows, nCols;
			m_new_scanline.clear();
      		MOOSVectorFromString(msg.GetString(), m_new_scanline, nRows, nCols);

      		if(m_new_scanline.size() != m_size_scanline){
      			m_img_sonar.create(2*m_new_scanline.size(),2*m_new_scanline.size(),CV_8UC(1));
      			m_img_sonar.setTo(Scalar(255));
      		}
      		m_size_scanline = m_new_scanline.size();
      		m_new_data = true;
        }

		// **************** CONFIG DATA ************************
        else if(msg.GetKey() == m_sonar_minking_contrast_keyname)
			m_sonar_contrast_miniking = msg.GetDouble();
        else if(msg.GetKey() == m_sonar_micron_contrast_keyname)
			m_sonar_contrast_micron = msg.GetDouble();
        else if(msg.GetKey() == m_sonar_contrast_keyname)
            m_sonar_contrast = msg.GetDouble();

        // **************** WALL DETECTION DATA ************************
        else if(msg.GetKey() == m_wall_detector_minking_keyname){
			double dist, bearing;
			MOOSValFromString(bearing, msg.GetString(), "bearing", true);
			MOOSValFromString(dist, msg.GetString(), "distance", true);
			m_wall_detector_distance_miniking.push_back(dist);
			m_wall_detector_bearing_miniking.push_back(bearing);

			if(m_wall_detector_distance_miniking.size()>100){
				m_wall_detector_distance_miniking.erase(m_wall_detector_distance_miniking.begin());
			}
		}
        else if(msg.GetKey() == m_wall_detector_micron_keyname){
			double dist, bearing;
			MOOSValFromString(bearing, msg.GetString(), "bearing", true);
			MOOSValFromString(dist, msg.GetString(), "distance", true);
			m_wall_detector_distance_micron.push_back(dist);
			m_wall_detector_bearing_micron.push_back(bearing);

			if(m_wall_detector_distance_micron.size()>100){
				m_wall_detector_distance_micron.erase(m_wall_detector_distance_micron.begin());
			}
		}
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
		Point pt1(m_size_scanline_micron, m_size_scanline_micron);
		Point pt2(m_size_scanline_micron + m_size_scanline_micron*cos(m_new_bearing_micron), m_size_scanline_micron+ m_size_scanline_micron*sin(m_new_bearing_micron));
		
		m_img_micron = Scalar(0,0,0);
		cvtColor(m_img_sonar_micron, m_img_micron, CV_GRAY2RGB);
		line(m_img_micron, pt1, pt2, Scalar(255, 0, 0));

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
	imshow("SONAR MICRON", m_img_micron);
    imshow("SONAR", m_img_sonar);
	
    imshow("CAMERA 1", m_img_camera_side);
    imshow("CAMERA 2", m_img_camera_bottom);

	imshow("WALL MICRON", m_img_wall_micron);
    imshow("WALL MINIKING", m_img_wall_miniking);

	waitKey(1);
    AppCastingMOOSApp::PostReport();
	return(true);
}

bool SensorViewer::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
    {
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
            else if (param == "VARIABLE_IMAGE_1_NAME") {
                m_camera_side_keyname = value;
                handled = true;
            }
            else if (param == "VARIABLE_IMAGE_2_NAME") {
                m_camera_bottom_keyname = value;
                handled = true;
            }

            if (!handled)
                reportUnhandledConfigWarning(orig);
        }
    }

    registerVariables();

	return(true);
}

void SensorViewer::registerVariables()
{
	AppCastingMOOSApp::RegisterVariables();

	// CAMERA DATA
    Register(m_camera_side_keyname, 0);
    Register(m_camera_bottom_keyname, 0);

	// SONAR DATA
    Register(m_sonar_raw_keyname, 0);
    Register(m_sonar_contrast_keyname, 0);

    Register(m_sonar_micron_keyname, 0);
    Register(m_sonar_micron_contrast_keyname, 0);

    Register(m_sonar_minking_keyname, 0);
    Register(m_sonar_minking_contrast_keyname, 0);

	// WALL DETECTOR
    Register(m_wall_detector_micron_keyname, 0);
    Register(m_wall_detector_minking_keyname, 0);
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

    m_msgs << m_nb_camera_bottom << " " << m_nb_camera_side;

  return(true);
}

void SensorViewer::AddSector(Mat &img_sonar, vector<double> scanline, double bearing, double &bearing_previous, double contrast){
	for(int r=1; r<scanline.size(); r++){
		double angle_diff = abs(2*atan(tan(abs(bearing - bearing_previous)/2)));
		double theta_begin = min(bearing, bearing_previous);

		for(double theta=theta_begin; theta<theta_begin + angle_diff; theta+=1.0/r){
			int x = round(scanline.size() + r*cos(theta));
			int y = round(scanline.size() + r*sin(theta));

            img_sonar.at<unsigned char>(y, x) = 255 - pow(10, scanline[r]/20.0)/pow(10, 150/20.0)*255*255*contrast;
		}
	}
	bearing_previous = bearing;
}

void SensorViewer::UpdateDistance(Mat &img_wall, vector<double> distance_tab, vector<double> bearing_tab){
	img_wall = Scalar(0);

	for(int i =0; i<distance_tab.size(); i++){
		int x = round(250+distance_tab[i]*cos(bearing_tab[i]));
		int y = round(250+distance_tab[i]*sin(bearing_tab[i]));
		circle(img_wall, Point(x, y), 5, 255);
	}
}
