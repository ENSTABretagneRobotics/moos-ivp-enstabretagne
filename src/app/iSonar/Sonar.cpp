#include <iterator>
#include "MBUtils.h"
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
	m_bPollSonar = true;

	if (!m_serial_thread.Initialise(listen_sonar_messages_thread_func, (void*)this))
	  MOOSFail("Sonar thread initialization error...\n");
}

Sonar::~Sonar()
{
	MOOSTrace("iSonar: stopping aquisition thread.\n");
	m_serial_thread.Stop();
	MOOSTrace("iSonar: finished.\n");
}
 
bool Sonar::OnNewMail(MOOSMSG_LIST &NewMail)
{
	MOOSMSG_LIST::iterator p;

	for(p = NewMail.begin() ; p != NewMail.end() ; p++)
	{
		CMOOSMsg &msg = *p;

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
		if ( msg.GetKey() == "SONAR_PARAMS" && msg.IsString() ) 
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
	m_iterations++;
	
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
	setlocale(LC_ALL, "C");
	list<string> sParams;
	m_MissionReader.EnableVerbatimQuoting(false);
	
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

			if(param == "SERIAL_PORT_NAME")
			{
			    MOOSTrace("iSonar: Using %s serial port\n", value.c_str());
			    m_portName = value;
			}
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
    }
	}
	else
		MOOSTrace("No configuration read.\n");

	bool portOpened = this->m_Port.Create(m_portName.c_str(), 115200);

	//this->m_Port.SetTermCharacter('\n');
	m_Port.Flush();

	m_timewarp = GetMOOSTimeWarp();

	RegisterVariables();
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
        
	return(true);
}
 
void Sonar::RegisterVariables()
{
	Register("SONAR_PARAMS", 0);
}
