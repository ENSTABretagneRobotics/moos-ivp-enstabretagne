#ifndef Modem_HEADER
#define Modem_HEADER

#include "seanetmsg.h"

#include <fstream>
#include <string.h>

#include "MOOS/libMOOS/App/MOOSInstrument.h"

#define MESSAGE_MAX_LENGTH 4

class Modem : public CMOOSInstrument
{
public:
	Modem();
	~Modem();

protected:
	bool OnNewMail(MOOSMSG_LIST &NewMail);
	bool Iterate();
	bool OnConnectToServer();
	void ModemTempoFunction();
	void ListenModemMessages();
	bool receiveMessage(std::string & message, double reception_timeout);
	bool OnStartUp();
	void RegisterVariables();


	bool SendModemConfigurationMessage(const SeaNetMsg & msg) {
		return (m_Port.Write(msg.data().data(), (int)msg.data().size()) == (int)msg.data().size());}

	private: // Configuration variables
	std::string 	m_portName;
	int 			m_baudrate_conf;
	int 			m_baudrate_comm;
	char 			m_modemNodeAddr; //always 85 for modem, setted in Ctor

	//Flags for configuration process
	bool 			m_bModemConfigurationRequired;
	bool 			m_bIsAlive;
	bool 			m_bSentCfg;
	bool 			m_bGetVersionData;
	bool 			m_bGetBBUserData;
	bool 			m_bGetFpgaVersionData;
	bool 			m_bGetFirstPgrAck;
	bool 			m_bGetSecondPgrAck;
	bool 			m_bGetThirdPgrAck;
	bool 			m_bMtReBootHasBeenSent;
	bool 			m_bModemConfiguratonComplete;
	//Config received by the modem that contain master/slave config
	std::string 	m_BBUserData;
	int 					m_iModemRoleRequired;
	//Value of timeout applied when sending some configuration commands to the modem
	unsigned int 	m_uiTimeoutUS;


	private: // State variables
	unsigned int	m_iterations;
	double				m_timewarp;

	//Thread created for modem configuration
	CMOOSThread 	m_serial_thread_conf;
	static bool listen_Modem_config_thread_func(void *pModemObject)
	{
		Modem* pModem = static_cast<Modem*> (pModemObject);
		if (pModem)
		{
			std::cout<<"Configuration thread launched, listenning on serial port for conf process"<<std::endl;
			pModem->ListenModemMessages();
			return true;
		}
		else return false;
	}

	//Thread created for timeouts, probably not the right way to do but it works like a charm
	CMOOSThread 	m_serial_thread_tempo;
	static bool listen_Modem_tempo_thread_func(void *pModemObject)
	{
		Modem* pModem = static_cast<Modem*> (pModemObject);
		if (pModem)
		{
			std::cout<<"Timeout configuration thread launched"<<std::endl;
			pModem->ModemTempoFunction();
			return true;
		}
		else return false;
	}
};
#endif
