/************************************************************/
/*    FILE: SensorViewer.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#ifndef SensorViewer_HEADER
#define SensorViewer_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "SonarMapping.h"
#include "CameraViewer.h"
#include "Viewer.h"

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

	private: // Configuration variables

    private:

        // State variables
        std::map<string, CameraViewer> mapCameras;
        std::map<string, SonarMappingScanline> mapSonarsScanline;
        std::map<string, SonarMappingDistance> mapSonarsDistance;

        Viewer mViewer;



};

#endif
