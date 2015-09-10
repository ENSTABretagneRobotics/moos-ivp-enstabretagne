#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Sonar.h"
#include "seanetmsg.h"


using namespace std;

Sonar::Sonar()
{
	m_iterations = 0;
	m_timewarp   = 1;

	m_bNoParams = true;
	m_bSentCfg = false;

	m_bSonarReady = false;
	m_bPollSonar = false;
	m_bIsPowered = false;

	if (!m_serial_thread.Initialise(listen_sonar_messages_thread_func, (void*)this))
	{
	  reportRunWarning("Sonar thread initialization error...\n");
	  MOOSFail("Sonar thread initialization error...\n");
	}
}

Sonar::~Sonar()
{
	MOOSTrace("iSonar: stopping aquisition thread.\n");
	m_serial_thread.Stop();
	MOOSTrace("iSonar: finished.\n");
}

bool Sonar::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
	MOOSMSG_LIST::iterator p;

	for(p = NewMail.begin() ; p != NewMail.end() ; p++)
	{
		CMOOSMsg &msg = *p;
    	string key    = msg.GetKey();

		#if 0 // Keep these around just for template
		string key   = msg.GetKey();
		string comm  = msg.GetCommunity();
		double dval  = msg.GetDouble();
		string sval  = msg.GetString();
		string msrc  = msg.GetSource();
		double mtime = msg.GetTime();
		bool   mdbl  = msg.IsDouble();
		bool   mstr  = msg.IsString();
		#endif

		// Mise à jour des paramètres du sonar
		if ( key == "SONAR_PARAMS" && msg.IsString() )
		{
		  string msg_val = msg.GetString();
		  // Le message est de la forme "Range=25,Gain=45,Continuous=true"
		  double dVal=0.0; int iVal; bool bVal;
		  if (MOOSValFromString(dVal, msg_val, "Range", true))
		    m_msgHeadCommand.setRange(dVal);
		  if (MOOSValFromString(iVal, msg_val, "nBins", true))
		    m_msgHeadCommand.setNbins(iVal);
		  if (MOOSValFromString(dVal, msg_val, "AngleStep", true))
	      	m_msgHeadCommand.setAngleStep(dVal);
		  if (MOOSValFromString(bVal, msg_val, "Continuous", true))
	     	m_msgHeadCommand.setContinuous(bVal);
		  if (MOOSValFromString(dVal, msg_val, "Gain", true))
		    m_msgHeadCommand.setGain(dVal);
		  if (MOOSValFromString(dVal, msg_val, "LeftLimit", true)){
		    m_msgHeadCommand.setLeftLimit(dVal); cout << "limite gauche " << dVal << endl;}
		  if (MOOSValFromString(dVal, msg_val, "RightLimit", true)){
		    m_msgHeadCommand.setRightLimit(dVal); cout << "limite droite " << dVal << endl;}
		  // Envoi de la commande au sondeur
		  // TODO: vérifier que le CMOOSSerialPort est bien thread safe. Sinon, rajouter un mutex
		  SendSonarMessage(m_msgHeadCommand);
		}
		else if ( key == "SONAR_POLL")
		{
			m_bPollSonar = (msg.GetDouble())?true:false;
			if (m_bSonarReady && m_bSentCfg && !m_bNoParams)
			{
				SeaNetMsg_SendData msg_SendData;
				msg_SendData.setTime(MOOSTime());
				SendSonarMessage(msg_SendData);
				retractRunWarning("Sonar not initialized!");
			}
			else
			{
				reportRunWarning("Sonar not initialized!");
			}
		}
		else if ( key == "POWERED_SONAR")
		{
			if (msg.GetDouble() == 0)
			{
				m_bIsPowered = false;
				m_bNoParams = true;
				m_bSentCfg = false;

				m_bSonarReady = false;
				m_bPollSonar = false;
				m_serial_thread.Stop();
			}
			else
			{
				m_bIsPowered = true;
				Initialization();
			}
		}
	    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
	      reportRunWarning("Unhandled Mail: " + key);
	}

	return(true);
}

bool Sonar::OnConnectToServer()
{
	// register for variables here
	// possibly look at the mission file?
	// m_MissionReader.GetConfigurationParam("Name", <string>);
	// m_Comms.Register("VARNAME", 0);

	RegisterVariables();
	return(true);
}

bool Sonar::Iterate()
{
  	AppCastingMOOSApp::Iterate();
	m_iterations++;
  	AppCastingMOOSApp::PostReport();
	return(true);
}

void Sonar::ListenSonarMessages()
{
  const int buf_size = 512;
  char buf[buf_size];
  string sBuf;

  union HeadInf {
    char c;
    struct {
      int InCenter : 1;
      int Centered : 1;
      int Motoring : 1;
      int MotorOn  : 1;
      int Dir : 1;
      int InScan : 1;
      int NoParams : 1;
      int SentCfg : 1;
    } bits;
  } headInf;

  while (!m_serial_thread.IsQuitRequested())
  {
    int msg_size = 0;
    int needed_len = SeaNetMsg::numberBytesMissing(sBuf, msg_size);

    if (needed_len == SeaNetMsg::mrNotAMessage)
    {
      // Remove first character if the header cannot be decoded
      sBuf.erase(0,1);
    }
    else if (needed_len > 0)
    {
      // Read more data as needed
      int nb_read = m_Port.ReadNWithTimeOut(buf, needed_len);
      sBuf.append(buf, nb_read);
    }
    else if (needed_len == 0)
    {
      // Process message
      // cout << "Found message " << SeaNetMsg::detectMessageType(sBuf) << endl;
      SeaNetMsg snmsg(sBuf);
      //cout << "Created message with type " << snmsg.messageType() << endl;
      //snmsg.print_hex();

      if (snmsg.messageType() == SeaNetMsg::mtAlive) {
        headInf.c = snmsg.data().at(20);

        m_bNoParams = headInf.bits.NoParams;
        m_bSentCfg = headInf.bits.SentCfg;

        // Sonar polling was enabled but Sonar was not ready...
        // ...now that sonar is ready, start to poll.
        if (!m_bSonarReady && m_bPollSonar && !m_bNoParams && m_bSentCfg) {
          cout << "Sonar is now ready, initiating scanline polling." << endl;
          SeaNetMsg_SendData msg_SendData;
          msg_SendData.setTime(MOOSTime());
          SendSonarMessage(msg_SendData);
          Notify("SONAR_CONNECTED", "true");
        }

        // Update m_bSonarReady
        m_bSonarReady = (!m_bNoParams) && m_bSentCfg;
      }

      if (snmsg.messageType() == SeaNetMsg::mtHeadData)
      {
	      const SeaNetMsg_HeadData * pHdta = reinterpret_cast<SeaNetMsg_HeadData*> (&snmsg);

	      // MOOSDB raw data
	      vector<int> vScanline;
	      for (int k=0; k<pHdta->nBins(); ++k)
        	vScanline.push_back( pHdta->scanlineData()[k] );

	      stringstream ss;
	      ss << "bearing=" << pHdta->bearing() << ",";
	      ss << "ad_interval=" << pHdta->ADInterval_m() << ",";
	      ss << "scanline=";
	      Write(ss, vScanline);
	      Notify("SONAR_RAW_DATA", ss.str());

	      stringstream ss2;
	      ss2 << "bearing=" << pHdta->bearing() << ","
	          << "distance=" << pHdta->firstObstacleDist(90, 0.5, 100.); // thd, min, max
	      Notify("SONAR_DISTANCE", ss2.str());

	      if (m_bSonarReady && m_bPollSonar)
	      {
	          SeaNetMsg_SendData msg_SendData;
	          msg_SendData.setTime(MOOSTime());
	          SendSonarMessage(msg_SendData);
	      }
      }
      sBuf.erase(0,msg_size);
    }
  }
}

bool Sonar::OnStartUp()
{
  	AppCastingMOOSApp::OnStartUp();
	setlocale(LC_ALL, "C");
	list<string> sParams;
	m_MissionReader.EnableVerbatimQuoting(false);
  if (!m_MissionReader.GetValue("SONAR_SERIAL_PORT",m_portName))
    reportConfigWarning("No SONAR_SERIAL_PORT config found for " + GetAppName());

	if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
	{
    MOOSTrace("iSonar: Reading configuration\n");
    list<string>::iterator p;
    for(p = sParams.begin() ; p != sParams.end() ; p++)
    {
			string original_line = *p;
			string param = stripBlankEnds(toupper(biteString(*p, '=')));
			string value = stripBlankEnds(*p);

			MOOSTrace(original_line);

			if(MOOSStrCmp(param, "RANGE"))
			    m_msgHeadCommand.setRange(atof(value.c_str()));
			if(MOOSStrCmp(param, "NBINS"))
			    m_msgHeadCommand.setNbins(atoi(value.c_str()));
			if(MOOSStrCmp(param, "ANGLESTEP"))
			    m_msgHeadCommand.setAngleStep(atof(value.c_str()));
			if(MOOSStrCmp(param, "CONTINUOUS"))
			    m_msgHeadCommand.setContinuous(MOOSStrCmp(value,"true"));
			if(MOOSStrCmp(param, "GAIN"))
			    m_msgHeadCommand.setGain(atof(value.c_str()));
			if(MOOSStrCmp(param, "LEFTLIMIT"))
			    m_msgHeadCommand.setLeftLimit(atof(value.c_str()));
			if(MOOSStrCmp(param, "RIGHTLIMIT"))
			    m_msgHeadCommand.setRightLimit(atof(value.c_str()));
      if(MOOSStrCmp(param, "POWERED_AT_START"))
          m_bIsPowered = MOOSStrCmp(value.c_str(),"TRUE");
    }
	}
	else
		MOOSTrace("No configuration read.\n");

	bool portOpened = this->m_Port.Create(m_portName.c_str(), 115200);

	//this->m_Port.SetTermCharacter('\n');
	m_Port.Flush();

	m_timewarp = GetMOOSTimeWarp();

	RegisterVariables();

  	if (m_bIsPowered)
  		Initialization();

	return(true);
}
void Sonar::Initialization(void)
{

	m_serial_thread.Start();
	//////
	SeaNetMsg_ReBoot msg_ReBoot;

	MOOSPause(50);
	SendSonarMessage(msg_ReBoot);

	MOOSPause(1000);
	SendSonarMessage(SeaNetMsg_SendVersion());

	MOOSPause(50);
	SendSonarMessage(SeaNetMsg_SendBBUser());

	MOOSPause(50);
	bool sonarReady = SendSonarMessage(m_msgHeadCommand);

	Notify("SONAR_CONNECTED", "false");

	//////
}

void Sonar::RegisterVariables()
{
  	AppCastingMOOSApp::RegisterVariables();
	Register("SONAR_PARAMS", 0);
	Register("SONAR_POLL", 0);
	Register("POWERED_SONAR", 0);
}
//------------------------------------------------------------
// Procedure: buildReport()

bool Sonar::buildReport()
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
    m_msgs << "==============================================================\n";
    m_msgs << "iSonarStatus :                                                \n";
    m_msgs << "==============================================================\n";


    ACTable actab(4);
    actab << "SerialPort | Baudrate | Is Powered | Thread Running ";
    actab.addHeaderLines();
    string sonarIsPowered = (m_bIsPowered)?"yes":"no";
    string sonarThreadIsRunning = (m_serial_thread.IsThreadRunning())?"yes":"no";
    actab << m_portName << m_Port.GetBaudRate() << sonarIsPowered << sonarThreadIsRunning;
    m_msgs << actab.getFormattedString();
    m_msgs << "\n==============================================================\n";
    ACTable actab2(4);
    actab2 << "Has params | Sent cfg | Is ready | Is polling";
    actab2.addHeaderLines();
    string sonarHasParams = (m_bNoParams)?"no":"yes";
    string sonarSentCfg = (m_bSentCfg)?"yes":"no";
    string sonarIsReady = (m_bSonarReady)?"yes":"no";
    string sonarIsPolling = (m_bPollSonar)?"yes":"no";
    actab2 << sonarHasParams << sonarSentCfg << sonarIsReady << sonarIsPolling;
    m_msgs << actab2.getFormattedString();


  return true;
}
