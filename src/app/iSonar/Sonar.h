#ifndef Sonar_HEADER
#define Sonar_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#ifdef _WIN32
  #include "MOOS/libMOOS/Utils/MOOSNTSerialPort.h"
#else
  #include "MOOS/libMOOS/Utils/MOOSLinuxSerialPort.h"
#endif

#include "seanetmsg.h"

#include <fstream>


class Sonar : public AppCastingMOOSApp
{
	public:
		Sonar();
		~Sonar();

	protected:
		bool OnNewMail(MOOSMSG_LIST &NewMail);
		bool Iterate();
		bool OnConnectToServer();
		bool OnStartUp();
		void RegisterVariables();
		void Initialization();
				
		void ListenSonarMessages();
			   
		bool SendSonarMessage(const SeaNetMsg & msg) {return (m_Port.Write(msg.data().data(), (int)msg.data().size()) == (int)msg.data().size());}
  
  	protected: // Standard AppCastingMOOSApp functions to overload
    	bool buildReport();

	private: // Configuration variables
		std::string m_portName;
		SeaNetMsg_HeadCommand m_msgHeadCommand;


	private: // State variables
	    #ifdef _WIN32
	      CMOOSNTSerialPort m_Port;
	    #else
	      CMOOSLinuxSerialPort m_Port;
	    #endif
		unsigned int			m_iterations;
		double			        m_timewarp;
		bool m_bNoParams;
		bool m_bSentCfg;
				
		bool m_bSonarReady;
		bool m_bPollSonar;
		bool m_bIsPowered;
						
		CMOOSThread m_serial_thread;

		static bool listen_sonar_messages_thread_func(void *pSonarObject) {
			Sonar* pSonar = static_cast<Sonar*> (pSonarObject);
			if (pSonar) 
			{
				pSonar->ListenSonarMessages();
				return true;
			}
			else return false;
		}
};

#endif 
