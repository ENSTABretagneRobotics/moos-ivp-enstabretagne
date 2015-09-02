#ifndef Sonar_HEADER
#define Sonar_HEADER

#include "seanetmsg.h"

#include <fstream>

#include "MOOS/libMOOS/App/MOOSInstrument.h"

class Sonar : public CMOOSInstrument
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
				
		void ListenSonarMessages();
			   
		bool SendSonarMessage(const SeaNetMsg & msg) {return (m_Port.Write(msg.data().data(), (int)msg.data().size()) == (int)msg.data().size());}

	private: // Configuration variables
		std::string m_portName;
		SeaNetMsg_HeadCommand m_msgHeadCommand;


	private: // State variables
		unsigned int			m_iterations;
		double			        m_timewarp;
		bool m_bNoParams;
		bool m_bSentCfg;
				
		bool m_bSonarReady;
		bool m_bPollSonar;
						
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
