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
#include <vector>

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
		void registerVariables();
		bool buildReport();

	private: // Fonctions
		void AddSector(cv::Mat &img_sonar, vector<double> scanline, double bearing, double &bearing_previous, double contrast);
		void UpdateDistance(cv::Mat &img_wall, vector<double> distance_tab, vector<double> bearing_tab);

	private: // Configuration variables

	private: // State variables

		cv::Mat m_img_camera_bottom;
		cv::Mat m_img_camera_side;

		cv::Mat m_img_sonar_micron;
		cv::Mat m_img_sonar_miniking;
		cv::Mat m_img_sonar;

		cv::Mat m_img_micron;
		cv::Mat m_img_miniking;

		cv::Mat m_img_wall_miniking;
		cv::Mat m_img_wall_micron;

		// SONAR DATAS
	    std::vector<double> m_new_scanline_micron;
	    std::vector<double> m_new_scanline_miniking;
	    std::vector<double> m_new_scanline;
    	double m_new_bearing_micron, m_old_bearing_micron;
    	double m_new_bearing_miniking, m_old_bearing_miniking;
    	double m_new_bearing, m_old_bearing;

    	bool m_new_data_micron;
    	bool m_new_data_miniking;
    	bool m_new_data;

    	int m_size_scanline_micron;
    	int m_size_scanline_miniking;
    	int m_size_scanline;

    	double m_sonar_contrast_micron;
    	double m_sonar_contrast_miniking;
    	double m_sonar_contrast;

    	// WALL DETECTOR DATA
    	std::vector<double> m_wall_detector_distance_micron;
    	std::vector<double> m_wall_detector_bearing_micron;

    	std::vector<double> m_wall_detector_distance_miniking;
    	std::vector<double> m_wall_detector_bearing_miniking;
};

#endif
