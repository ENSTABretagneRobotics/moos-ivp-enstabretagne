/************************************************************/
/*    NAME:Elouan Autret                                    */
/*    ORGN: Ensta Bretagne                                  */
/*    FILE: LidarMove.h                                     */
/*    DATE: Movember 03, 2015                               */
/************************************************************/

#ifndef LidarMove_HEADER
#define LidarMove_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class LidarMove : public AppCastingMOOSApp
{
 public:
   LidarMove();
   ~LidarMove() {};

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();

 private: // Configuration variables
   int m_AngleMin;
   int m_AngleMax;
   int m_ModeMove;
   int m_ModeComLidar;
   std::string m_UrlServo;
   std::string m_UrlLidar;
   int m_ServoId;
   int m_IndexServo;
   int m_BaudrateLidar;
   int m_BaudrateServo;
   bool m_Debug;
   int m_ServoSpeed;
 private: // State variables
   int m_GoalPos[2];
   int m_index;
};

#endif 
