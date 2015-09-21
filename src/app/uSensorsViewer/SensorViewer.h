/************************************************************/
/*    FILE: SensorViewer.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH:
/*    DATE: 2015
/************************************************************/

#ifndef SensorViewer_HEADER
#define SensorViewer_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

//#include "Ciscrea_Images/Ciscrea_Images.h"

using namespace std;

class SensorViewer : public AppCastingMOOSApp
{
	public:
		SensorViewer();
		~SensorViewer();

	protected:
		bool OnNewMail(MOOSMSG_LIST &NewMail);
		bool Iterate();
		bool OnConnectToServer();
		bool OnStartUp();
		void RegisterVariables();
		bool buildReport();

	private: // Fonctions
		void AddSector(cv::Mat &img_sonar, vector<double> scanline, double bearing, double &bearing_previous);

	private: // Configuration variables

	private: // State variables

		cv::Mat m_img_camera_bottom, m_img_camera_front;
		cv::Mat m_img_sonar_micron, m_img_sonar_miniking;

		// SONAR DATAS
	    std::vector<double> m_new_scanline_micron;
	    std::vector<double> m_new_scanline_miniking;
    	double m_new_bearing_micron, m_old_bearing_micron;
    	double m_new_bearing_miniking, m_old_bearing_miniking;

    	bool m_new_data_micron;
    	bool m_new_data_miniking;

    	int m_size_scanline_micron;
    	int m_size_scanline_miniking;
};

#endif
