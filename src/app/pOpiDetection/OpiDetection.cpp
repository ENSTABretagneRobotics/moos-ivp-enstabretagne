/************************************************************/
/*    FILE: OpiDetection.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Guilherme Schvarcz Franco, Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "OpiDetection.h"

using namespace cv;
using namespace std;

//---------------------------------------------------------
// Constructor

OpiDetection::OpiDetection() : 
     show_process(true),
     message_name("Buoy"),
     path_save(""),
     folder_name_pattern("/DATASET_%F_%H-%M-%S"),
     image_name_pattern("/image_%H_%M_%S.jpg")
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool OpiDetection::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

    #if 0 // Keep these around just for template
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString(); 
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif

    if(msg.GetKey() == image_name)
      memcpy(img.data, msg.GetBinaryData(), img.rows*img.step);

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool OpiDetection::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool OpiDetection::Iterate()
{
  AppCastingMOOSApp::Iterate();

  detect(img);

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool OpiDetection::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  setlocale(LC_ALL, "C");
  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  sParams.reverse();
  for(p = sParams.begin() ; p != sParams.end() ; p++)
  {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;
    bool handled = false;

    if(param == "VARIABLE_IMAGE_NAME")
    {
      image_name = value;
      handled = true;
    }

    else if(param == "VARIABLE_WHEN_FOUND")
    {
      message_name = value;
      handled = true;
    }

    else if(param == "SHOW_PROCESS")
    {
      show_process = (value == "true");
      handled = true;
    }

    else if(param == "IMAGE_NAME_PATTERN")
    {
      image_name_pattern = "/" + value;
      handled = true;
    }

    else if(param == "FOLDER_NAME_PATTERN")
    {
      folder_name_pattern = "/" + value;
      handled = true;
    }

    else if(param == "SAVE_IN_FOLDER")
    {
      path_save = value;
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  char folder_name[80];
  time_t now = time(0);
  struct tm tstruct;
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  strftime(folder_name, sizeof(folder_name), folder_name_pattern.c_str(), &tstruct);
  path_save += folder_name;
  mkdir(path_save.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  registerVariables();  
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void OpiDetection::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool OpiDetection::buildReport() 
{
  #if 0 // Keep these around just for template
    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif

  return true;
}

//---------------------------------------------------------
// Procedure: detect

void OpiDetection::detect(Mat img)
{
  Mat imgHSV, imgThr, imgThr2, eqHSV;
  img = img(Rect(10, 10, img.cols-20, img.rows-20));
  blur(img, img, Size(3,3));
  blur(img, img, Size(3,3));
  cvtColor(img, imgHSV, CV_RGB2HSV);
  vector<Mat> channels;
  split(imgHSV,channels);

  for(int i = 0 ; i < channels.size() ; i++)
    equalizeHist(channels[i],channels[i]);
  merge(channels, eqHSV);

  inRange(imgHSV, Scalar(95, 100, 0), Scalar(200,200,255), imgThr);
  inRange(eqHSV, Scalar(250, 240, 250), Scalar(255,255,255), imgThr2);

  Moments m1 = moments(imgThr);
  Point2f centerDetection(0, 0);

  int found = 0;
  if(m1.m00 != 0)
  {
    centerDetection.x = m1.m10/m1.m00;
    centerDetection.y = m1.m01/m1.m00;
    found += 1;
    if(show_process)
    {
      Point center1(m1.m10/m1.m00,m1.m01/m1.m00);
      circle(img, center1, 5, Scalar(0, 0, 255));
    }
  }

  Moments m2 = moments(imgThr2);

  if(m2.m00 != 0)
  {
    centerDetection.x += m2.m10/m2.m00;
    centerDetection.y += m2.m01/m2.m00;
    if(found == 1)
    {
      centerDetection.x /= 2.;
      centerDetection.y /= 2.;
    }

    found += 2;

    if(show_process)
    {
      Point center2(m2.m10/m2.m00,m2.m01/m2.m00);
      circle(img, center2, 5, Scalar(255, 0, 0));
    }
  }

  centerDetection.x -= img.cols/2.;
  centerDetection.y -= img.rows/2.;

  if(found > 0)
  {
    char name[80];
    time_t now = time(0);
    struct tm tstruct;
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(name, sizeof(name), image_name_pattern.c_str(), &tstruct);
    imwrite(path_save+name, img);

    char message[80];
    sprintf(message, "x=%f,y=%f,img_w=%d,img_h=%d", centerDetection.x, -centerDetection.y, img.cols, img.rows);

    m_Comms.Notify(message_name, message);
  }

  else
    m_Comms.Notify(message_name, "false");

  if(show_process)
  {
    imshow("Original", img);
    imshow("Result", imgThr);
    imshow("Result", imgThr);
    imshow("Result2", imgThr2);
    imshow("imghsv", imgHSV);
    imshow("Heq", eqHSV);
    waitKey(20);
  }
}