/************************************************************/
/*    FILE: Camera.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH:
/*    DATE: 2015
/************************************************************/
#include <iterator>
#include <iostream>
#include <iomanip>
#include "MBUtils.h"
#include "ACTable.h"
#include "Camera.h"

/**
 * \fn
 * \brief Constructeur de l'application MOOS
 */

Camera::Camera()
{
  m_iterations = 0;
  m_timewarp   = 1;
}

/**
 * \fn
 * \brief Destructeur de l'instance de l'application
 */

Camera::~Camera()
{
}

/**
 * \fn
 * \brief Méthode appelée lorsqu'une variable de la MOOSDB est mise à jour
 * N'est appelée que si l'application s'est liée à la variable en question
 */

bool Camera::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
  return(true);
}

/**
 * \fn
 * \brief Méthode appelée dès que le contact avec la MOOSDB est effectué
 */

bool Camera::OnConnectToServer()
{
  RegisterVariables();
  Register("Image", 0.0);

  return(true);
}

/**
 * \fn
 * \brief Méthode appelée automatiquement périodiquement
 * Implémentation du comportement de l'application
 */

bool Camera::Iterate()
{
  AppCastingMOOSApp::Iterate();
  Mat m_capture_frame(LARGEUR_IMAGE_CAMERA, HAUTEUR_IMAGE_CAMERA, CV_8UC(3));

  if(m_vc_v4l2.read(m_capture_frame))
  {
    if(m_inverser_image)
      flip(m_capture_frame, m_capture_frame, -1);

    Notify((char*)(m_image_name).c_str(), (void*)m_capture_frame.data, 3 * LARGEUR_IMAGE_CAMERA * HAUTEUR_IMAGE_CAMERA, MOOSLocalTime());
    //imwrite("test.jpeg", m_capture_frame);

    if(m_affichage_image)
    {
      imshow(m_display_name, m_capture_frame);
      waitKey(10);
    }
    retractRunWarning("No frame grabbed.");
  }
  else
    reportRunWarning("No frame grabbed.");
    // MOOSDebugWrite("No frame grabbed.");

  AppCastingMOOSApp::PostReport();
  return(true);
}

/**
 * \fn
 * \brief Méthode appelée au lancement de l'application
 */

bool Camera::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
  setlocale(LC_ALL, "C");
  int identifiant_camera = -1;
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

      if(param == "IDENTIFIANT_CV_CAMERA")
        identifiant_camera = atoi((char*)value.c_str());
      if(param == "LARGEUR_IMAGE_CAMERA")
        LARGEUR_IMAGE_CAMERA = atoi((char*)value.c_str());
      if(param == "HAUTEUR_IMAGE_CAMERA")
        HAUTEUR_IMAGE_CAMERA = atoi((char*)value.c_str());

      if(param == "VARIABLE_IMAGE_NAME")
      {
        m_image_name = value;
        m_display_name = m_image_name;
      }

      if(param == "DISPLAY_IMAGE")
        m_affichage_image = (value == "true");

      if(param == "INVERT_IMAGE")
        m_inverser_image = (value == "true");
    }
  }

  m_timewarp = GetMOOSTimeWarp();

//  SetAppFreq(20, 400);
//  SetIterateMode(REGULAR_ITERATE_AND_COMMS_DRIVEN_MAIL);

  if(identifiant_camera == -1)
  {
    reportConfigWarning("Aucun identifiant de caméra reconnu");
    // cout << "Aucun identifiant de caméra reconnu" << endl;
    return false;
  }

  char buff[100];
  sprintf(buff, "/dev/video%d", identifiant_camera);
  string device_name = buff;

  if(!m_vc_v4l2.open(device_name, LARGEUR_IMAGE_CAMERA, HAUTEUR_IMAGE_CAMERA))
    return false;

  if(m_affichage_image)
    namedWindow(m_display_name, CV_WINDOW_NORMAL);

  RegisterVariables();
  setlocale(LC_ALL, "");
  return(true);
}

/**
 * \fn
 * \brief Inscription de l'application à l'évolution de certaines variables de la MOOSDB
 */

void Camera::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
}

bool Camera::buildReport()
{
  #if 0 // Keep these around just for template
    m_msgs << "============================================ \n";
    m_msgs << "File:                                        \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif
    m_msgs << "============================================ \n";
    m_msgs << GetAppName() << "Status: \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Img Name | Display Name | Inverted | width*heignt";
    actab.addHeaderLines();
    string imgInverted = (m_inverser_image)?"yes":"no";
    char imgSize[50];
    sprintf(imgSize,"%d*%d",LARGEUR_IMAGE_CAMERA,HAUTEUR_IMAGE_CAMERA);
    actab << m_image_name << m_display_name << imgInverted << imgSize;
    m_msgs << actab.getFormattedString();


  return(true);
}
