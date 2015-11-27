/************************************************************/
/*    FILE: OpiDetection.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco, Simon Rohou
/*    DATE: 2015
/************************************************************/

#ifndef OpiDetection_HEADER
#define OpiDetection_HEADER

#include <sys/stat.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class OpiDetection : public AppCastingMOOSApp
{
  public:
    OpiDetection();
    ~OpiDetection() {};

  protected: // Standard MOOSApp functions to overload  
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

  protected: // Standard AppCastingMOOSApp functions to overload 
    bool buildReport();
    void registerVariables();

  protected: // OpiDetection functions
    void detect(cv::Mat img);

  private: // Configuration variables


  private: // State variables
    cv::Mat img;
    bool show_process;
    std::string image_name, 
                message_name, 
                image_name_pattern, 
                folder_name_pattern, 
                path_save;
};

#endif 
