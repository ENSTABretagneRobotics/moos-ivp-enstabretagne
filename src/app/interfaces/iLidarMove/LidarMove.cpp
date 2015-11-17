/************************************************************/
/*    NAME:Elouan Autret                                    */
/*    ORGN: Ensta Bretagne                                  */
/*    FILE: LidarMove.h                                     */
/*    DATE: Movember 03, 2015                               */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "LidarMove.h"
#include "serial_lidar.h"
#include "r_28_servo.h"
#include "dynamixel.h"
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;
ofstream fichier;
//---------------------------------------------------------
// Constructor

LidarMove::LidarMove()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool LidarMove::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
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

     if(key == "FOO") 
       cout << "great!";

     else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool LidarMove::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LidarMove::Iterate()
{
  AppCastingMOOSApp::Iterate();
  //loop
  int Moving;
  int CommStatus;
  setAngle(m_ServoId,m_GoalPos[m_index]);
  isMoving(m_ServoId,&Moving,&CommStatus); 
  int angle_i;
  getPosition(m_ServoId,&angle_i,&CommStatus);

  if( angle_i < m_GoalPos[m_index]+5 && angle_i > m_GoalPos[m_index]-5 )
  {
     // Change goal position
     if( m_index == 0 )
         m_index = 1;
     else
         m_index = 0;					
   }
  int lidar_value = read_distance();
  struct rusage usage;
  struct timeval start;
 
  long mtime, seconds, useconds;    
  gettimeofday(&start, NULL);
  seconds  = start.tv_sec;
  useconds = start.tv_usec;
 
  double tempsSystem = seconds + useconds*1E-6;
  
  double angle = angle_i*(300/1024.0)-150;

  if (m_Debug)
      printf("%f , %f , %d\n",tempsSystem,angle,lidar_value);
  fichier.precision(15);
  fichier << tempsSystem << ',' <<lidar_value <<  ',' << angle << endl;
  fichier.flush();

  Notify("LIDAR_DISTANCE", lidar_value);
  Notify("LIDAR_ANGLE", angle);
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LidarMove::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  m_AngleMin = 358;//config pi/4
  m_AngleMax = 666;
  m_ModeMove = 0;//0 angle by angle 1 fast
  m_ModeComLidar=0;//0 pwm
  m_IndexServo = 0;
  m_UrlLidar = "/dev/ttyACM0";
  m_ServoId = 1;
  m_BaudrateLidar=112500;
  m_BaudrateServo = 1;
  m_Debug = false;
  m_index=0;

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;

    

    bool handled = false;
    if(param == "ANGLE_MIN") {
      handled = true;
      m_AngleMin = atoi(value.c_str());
      if (!(handled = !(m_AngleMin > 1023 || m_AngleMin<0)))
         printf("Angle_Min not in range 0-1023\n");
    }
    else if(param == "ANGLE_MAX") {
      handled = true;
      m_AngleMax = atoi(value.c_str());
      if (!(handled = !(m_AngleMax > 1023 || m_AngleMax<0)))
         printf("Angle_Min not in range 0-1023\n");
    }
    else if(param == "MODE_MOVE") {//angle to angle or fast travel
      handled = true;
      m_ModeMove = atoi(value.c_str());
      if (!(handled = !(m_ModeMove > 1|| m_ModeMove<0)))
         printf("ModeMove not 0-1\n");
    }
    else if(param == "MODE_COM_LIDAR") {//deprecated
      handled = true;
      m_ModeComLidar = atoi(value.c_str());
      if (!(handled = !(m_ModeComLidar > 1|| m_ModeComLidar <0)))
         printf("ModeComLidar not  0-1\n");
    }
    else if(param == "INDEX_SERVO") {
      handled = true;
      m_IndexServo = atoi(value.c_str());
    }
    else if(param == "URL_LIDAR") {
      handled = true;
      m_UrlLidar = value.c_str();
    }
    else if(param == "SERVO_ID") {
      handled = true;
      m_ServoId = atoi(value.c_str());
    }
    else if(param == "BAUDRATE_LIDAR") {
      handled = true;
      m_BaudrateLidar = atoi(value.c_str());
    }
    else if(param == "BAUDRATE_SERVO") {
      handled = true;
      m_BaudrateServo = atoi(value.c_str());
    }
    else if(param == "DEBUG") {
      handled = true;
      m_Debug = atoi(value.c_str()) == 1;
    }
    else if(param == "SERVO_SPEED") {
      handled = true;
      m_ServoSpeed = atoi(value.c_str());
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }

  std::string filepath = "/home/elessog/data/test.txt";

  fichier.open(filepath.c_str(), ios::out | ios::trunc);
  if(!fichier)
  {
    cerr << "File could not be openned" << endl;
    exit(1);
  }
  
  registerVariables();	
  m_GoalPos[0] = m_AngleMin;
  m_GoalPos[1] = m_AngleMax;
  open_USB2Dyn(m_BaudrateServo,m_IndexServo);//servo
  start_serial(m_BaudrateLidar,m_UrlLidar.c_str());//lidar
  setSpeed(m_ServoId,m_ServoSpeed);
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void LidarMove::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool LidarMove::buildReport() 
{
  m_msgs << "============================================ \n";
  m_msgs << "File:                                        \n";
  m_msgs << "============================================ \n";

  ACTable actab(4);
  actab << "Alpha | Bravo | Charlie | Delta";
  actab.addHeaderLines();
  actab << "one" << "two" << "three" << "four";
  m_msgs << actab.getFormattedString();

  return(true);
}




