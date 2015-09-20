/**
 * \file Camera.h
 * \brief Classe Camera
 * \author Team CISSAU - Veni Vidi Vici (ENSTA Bretagne)
 * \version 0.1
 * \date Jun 5th 2013
 *
 * Application MOOS récupérant les données de la caméra orientée vers l'avant
 *
 */

#ifndef Camera_HEADER
#define Camera_HEADER

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#include "videograbv4l2.h"

using namespace std;
using namespace cv;

class Camera : public AppCastingMOOSApp
{
  public:
    Camera();
    ~Camera();

  protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();
    bool buildReport();

  private: // Configuration variables

  private: // State variables
    string      m_image_name, m_display_name;
    unsigned int  m_iterations;
    double      m_timewarp;
    bool      m_server;
    //VideoCapture  m_vc;
    VideoGrabV4L2 m_vc_v4l2;
    bool      m_affichage_image, m_inverser_image;

    int LARGEUR_IMAGE_CAMERA, HAUTEUR_IMAGE_CAMERA;
};

#endif
