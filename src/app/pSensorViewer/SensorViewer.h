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

	private: // Configuration variables

	private: // State variables
		unsigned int	m_iterations;
		double			m_timewarp;
		cv::Mat img1, img2;
		cv::Mat img_son_pol;
		cv::Mat img_sonar;
		cv::Mat img_son_cart;
		cv::Mat pol2cart_x;
		cv::Mat pol2cart_y;

		double heading;
		double heading_razor;
		double heading_ciscrea;

		double vx;
};

#endif
