/************************************************************/
/*    FILE: Modem.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Clément Aubry (from V. Drevelle iSonar App)
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "Modem.h"
#include "seanetmsg.h"


using namespace std;

Modem::Modem()
{
    m_iterations = 0;
    m_timewarp   = 1;

    m_baudrate_conf = 57600;
    m_baudrate_comm =  9600;

    m_modemNodeAddr = 85; //Always 85 for Micron Data Modems
    m_bModemConfigurationRequired = false;
    m_bIsAlive = false;
    m_bSentCfg = false;
    m_bGetVersionData = false;
    m_bGetBBUserData = false;
    m_bGetFpgaVersionData = false;
    m_bGetFirstPgrAck = false;
    m_bGetSecondPgrAck = false;
    m_bGetThirdPgrAck = false;
    m_bModemConfiguratonComplete = false;
    m_iModemRoleRequired = 0; //0 = master, 1 = slave
    m_BBUserData = "";
    m_uiTimeoutUS = 0;

    //Init threads
    if (!m_serial_thread_conf.Initialise(listen_Modem_config_thread_func, (void*)this))
        MOOSFail("iModem: Modem thread listen initialization error...\n");
    if (!m_serial_thread_tempo.Initialise(listen_Modem_tempo_thread_func, (void*)this))
        MOOSFail("iModem: Modem thread tempo initialization error...\n");
}

Modem::~Modem()
{
    MOOSTrace("iModem: stopping threads.\n");
    m_serial_thread_conf.Stop();
    m_serial_thread_tempo.Stop();
    MOOSTrace("iModem: finished.\n");
}

bool Modem::OnNewMail(MOOSMSG_LIST &NewMail)
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

        if ( msg.GetKey() == "MODEM_GET_ROLE" && msg.IsString() )
        {
            MOOSTrace("iModem: New Mail asking current modem configuration\n");
            Notify("MODEM_ROLE", (m_iModemRoleRequired)?"slave":"master");
        }
        if ( msg.GetKey() == "MODEM_CONFIGURATION_REQUIRED" && msg.IsString() )
        {
            m_iModemRoleRequired = -1;
            string msg_val = msg.GetString();
            if (strcmp("master",msg_val.c_str()))
                m_iModemRoleRequired = 1;
            if (strcmp("slave",msg_val.c_str()))
                m_iModemRoleRequired = 0;
            if (m_iModemRoleRequired == 0 || m_iModemRoleRequired == 1)
            {
                m_bModemConfigurationRequired = true;
                MOOSTrace("iModem: New Mail asking new modem configuration as [%s]\n",msg.GetString().c_str());
            }
            else
                MOOSTrace("iModem: ERROR : New Mail asking new modem configuration as [%s], ASKED CONFIGURATION UNRECOGNIZED\n",msg.GetString().c_str());

        }
        if ( msg.GetKey() == "MODEM_SEND_MESSAGE" && msg.IsString() )
        {
            if (!m_bModemConfigurationRequired && m_Port.GetBaudRate() == 9600)
            {
                string msgToSent = msg.GetString();
                m_Port.Write(msgToSent.c_str(), msgToSent.size());
                MOOSTrace("iModem: Message [%s] sent.\n",msgToSent.c_str());
                Notify("MODEM_EMISSION_TIME", MOOSTime());
                Notify("MODEM_MESSAGE_SENT", msgToSent);
            }
            else
                MOOSTrace("iModem: Cannot send message, modem could be in a configuration step or serial port baddly configured\n");
        }
    }
    return(true);
}

bool Modem::Iterate()
{
    string message;
    m_iterations++;
    if(m_bModemConfigurationRequired)
    {
        //Configuration du port série pour la config (baudrate = 57600)
        if (m_Port.GetBaudRate() == 9600)
        {
            //Strange code to put an int to a string...
            string tt;
            string ttt("BAUDRATE=");
            char numstr[21]; // enough to hold all numbers up to 64-bits
            sprintf(numstr, "%d", m_baudrate_conf);
            tt = ttt + numstr;
            // cout<<"str tt["<<tt<<"]"<<endl;

            STRING_LIST params;
            params.push_back(tt);
            m_Port.Close();
            MOOSTrace("iModem: Configuring %s serial port at baud rate %d\n", m_portName.c_str(),m_baudrate_conf);
            bool portOpened = m_Port.Configure(params);
            MOOSTrace("iModem: Configured\n");
            //m_Port.SetTermCharacter('\n');
            m_Port.Flush();
            MOOSPause(1000);
            m_timewarp = GetMOOSTimeWarp();
        }
        //On relance le thread d'écoute pour la conf s'il est éteint
        if (!m_serial_thread_conf.IsThreadRunning())
            m_serial_thread_conf.Start();
    }
    else
    {
        // cout << "en comm, baudrate = " <<m_Port.GetBaudRate()<< endl;
        if (m_serial_thread_conf.IsThreadRunning())
            m_serial_thread_conf.Stop();
        if (m_serial_thread_tempo.IsThreadRunning())
            m_serial_thread_tempo.Stop();

        //Configuration du port série pour la comm (baudrate = 9600)
        if (m_Port.GetBaudRate() == 57600)
        {
            //Strange code to put an int to a string...
            string tt;
            string ttt("BAUDRATE=");
            char numstr[21]; // enough to hold all numbers up to 64-bits
            sprintf(numstr, "%d", m_baudrate_comm);
            tt = ttt + numstr;
            // cout<<"str tt["<<tt<<"]"<<endl;

            STRING_LIST params;
            params.push_back(tt);
            m_Port.Close();
            MOOSTrace("iModem: Configuring %s serial port at baud rate %d\n", m_portName.c_str(),m_baudrate_comm);
            bool portOpened = m_Port.Configure(params);
            MOOSTrace("iModem: Configured\n");
            //m_Port.SetTermCharacter('\n');
            m_Port.Flush();
            MOOSPause(1000);
            m_timewarp = GetMOOSTimeWarp();
        }


        if(receiveMessage(message, 1))
        {
            Notify("MODEM_RECEPTION_TIME", MOOSTime());
            MOOSTrace("iModem: Receiving [%s]\n",message.c_str());
            Notify("MODEM_MESSAGE_RECEIVED", message);
        }
    }

    return(true);
}

bool Modem::OnConnectToServer()
{
    // register for variables here
    // possibly look at the mission file?
    // m_MissionReader.GetConfigurationParam("Name", <string>);
    // m_Comms.Register("VARNAME", 0);

    RegisterVariables();
    return(true);
}

void Modem::ModemTempoFunction()
{
    if (!m_serial_thread_tempo.IsQuitRequested() && m_uiTimeoutUS != 0)
    {
        MOOSPause(m_uiTimeoutUS);
        m_uiTimeoutUS = 0;
    }
}

void Modem::ListenModemMessages()
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

    while (!m_serial_thread_conf.IsQuitRequested())
    {
        int msg_size = 0;
        int needed_len = SeaNetMsg::numberBytesMissing(sBuf, msg_size);
        //Waiting a message with header '@' = 0x40
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
        else if (needed_len == 0) //We found a message with the good header
        {
            // Process message
            MOOSTrace("iModem: Received type %d mtMessage: \n",SeaNetMsg::detectMessageType(sBuf));
            SeaNetMsg snmsg(sBuf,m_modemNodeAddr);
            snmsg.print_hex(300);

            if (snmsg.messageType() == SeaNetMsg::mtAlive)
            {
                m_bIsAlive = true;
                headInf.c = snmsg.data().at(20);

                m_bSentCfg = headInf.bits.SentCfg;
                if (!m_bSentCfg) //mtAlive HeadInf = 0x40 => config has not been sent, have to send mtSendVersion
                {
                    MOOSTrace("iModem: Modem says by mtAlive that config has not been sent (mtAlive HEadInf.sentCfg = 0) \n");
                }
                else if (m_bSentCfg) //mtAlive HeadInf = 0xC0 => have to send first mtErasesector
                {
                    MOOSTrace("iModem: Modem says by mtAlive that config has been sent (mtAlive HEadInf.sentCfg = 1) \n");
                }
            }
            else if (snmsg.messageType() == SeaNetMsg::mtVersionData)
            {
                m_bGetVersionData = true;
                    MOOSTrace("iModem: Modem reply mtVersionData after receiving mtSendVersion\n");
            }
            else if (snmsg.messageType() == SeaNetMsg::mtBBUserData)
            {
                m_bGetBBUserData = true;
                MOOSTrace("iModem: Modem reply mtBBUserData after receiving mtSendBBUser\n");
                m_BBUserData = snmsg.data().substr(13,128);
                string modemRole = (m_BBUserData.at(49))?"slave":"master";
                string modemRoleRequired = (m_iModemRoleRequired)?"slave":"master";
                MOOSTrace("iModem: Modem is currently set to %s\n",modemRole.c_str());
                MOOSTrace("iModem: Modem configuration required as %s\n",modemRoleRequired.c_str());
                m_BBUserData.at(49) = m_iModemRoleRequired;
                MOOSTrace("iModem: Modem will now send mtFpgaVersionData\n");
            }
            else if (snmsg.messageType() == SeaNetMsg::mtFpgaVersionData)
            {
                m_bGetFpgaVersionData = true;
                MOOSTrace("iModem: Modem send mtFpgaVersionData\n");
                MOOSTrace("iModem: Modem will now send mtAlive with HeadInf = 0xC0\n");
            }
            else if (snmsg.messageType() == SeaNetMsg::mtPgrAck)
            {
                MOOSTrace("iModem: Modem send mtPgrAck\n");
                if(snmsg.data().at(13) == 0x01 &&snmsg.data().at(14) == 0x06 && snmsg.data().at(15) == 0x00)
                {
                    m_bGetFirstPgrAck = true;
                    MOOSTrace("iModem: Modem send FIRST mtPgrAck\n");
                }
                else if(snmsg.data().at(13) == 0x02 &&snmsg.data().at(14) == 0x00 && snmsg.data().at(15) == 0x03)
                {
                    m_bGetSecondPgrAck = true;
                    MOOSTrace("iModem: Modem send SECOND mtPgrAck\n");
                }
                else if(snmsg.data().at(13) == 0x02 &&snmsg.data().at(14) == 0x01 && snmsg.data().at(15) == 0x03)
                {
                    m_bGetThirdPgrAck = true;
                    MOOSTrace("iModem: Modem send THIRD mtPgrAck\n");
                }
            }
            sBuf.erase(0,msg_size);
        }
        /*
        * Tritech Micron Data Modem configuration Process (see CompteRenduModemTritech.pdf)
        *  1) Modem will sent mtAlive messages every seconds with headInf bit = 0x40
        *  2) Send mtSendVersion
        *  3) Receive mtVersionData
        *  4) Send mtSendBBUser
        *  5) Receive mtBBUserData //see CompteRenduModemTritech.pdf, this reception contain Modem Configuration
        *  6) Receive mtFpgaVersionData
        *  7) Modem will sent mtAlive messages every seconds with headInf bit = = 0xc0
        *  8) Send mtEraseSector
        *  9) Receive mtPgrAck
        * 10) Send first mtProgBlock //see CompteRenduModemTritech.pdf, this progBlock contain Modem Configuration (with point 5 datas))
        * 11) Receive mtPgrAck
        * 12) Send second mtProgBlock
        * 13) Receive mtPgrAck
        * 14) Send mtReBoot
        */
        if (m_bIsAlive && !m_bModemConfiguratonComplete)
        {
            if(m_bGetThirdPgrAck && m_bMtReBootHasBeenSent && m_bSentCfg)
            {
                //Config has been done
                MOOSTrace("iModem: Configuration done.\n");
                m_bModemConfiguratonComplete = true;
                m_bModemConfigurationRequired = false;
                Notify("MODEM_CONFIGURATION_COMPLETE", true);
            }
            if(!m_bGetVersionData && m_uiTimeoutUS == 0)
            {
                //Send mtSendVersion
                SeaNetMsg_SendVersion msg_SendVersion(m_modemNodeAddr);
                SendModemConfigurationMessage(msg_SendVersion);
                MOOSTrace("iModem: Sending mtSendVersion : ");
                msg_SendVersion.print_hex(200);
                m_uiTimeoutUS = 600;
                m_serial_thread_tempo.Start();
            }
            else if(m_bGetVersionData && !m_bGetBBUserData && m_uiTimeoutUS == 0)
            {
                //Send mtSendBBUser
                SeaNetMsg_SendBBUser msg_SendBBUser(m_modemNodeAddr);
                SendModemConfigurationMessage(msg_SendBBUser);
                MOOSTrace("iModem: Sending mtSendBBUser : ");
                msg_SendBBUser.print_hex(200);
                m_uiTimeoutUS = 600;
                m_serial_thread_tempo.Start();
            }
            else if(m_bGetVersionData && m_bGetBBUserData &&
                    m_bGetFpgaVersionData && m_bSentCfg &&
                    !m_bGetFirstPgrAck && m_uiTimeoutUS == 0)
            {
                //Send mtEraseSector
                SeaNetMsg_EraseSector msg_EraseSector(m_modemNodeAddr, 0x06, 0x00);
                SendModemConfigurationMessage(msg_EraseSector);
                MOOSTrace("iModem: Sending mtEraseSector : ");
                msg_EraseSector.print_hex(200);
                m_uiTimeoutUS = 3000;
                m_serial_thread_tempo.Start();//A timeout of 3 seconds can be set. If the mtPgrAck is not received within this timeout period then re-send the mtEraseSector
            }
            else if(m_bGetFirstPgrAck && !m_bGetSecondPgrAck && !m_bGetThirdPgrAck && m_uiTimeoutUS == 0)
            {
                //Send first mtProgBlock
                string configMsg(m_BBUserData);
                // configMsg.append(m_BBUserData);
                SeaNetMsg_ProgBlock msg_ProgBlock(m_modemNodeAddr, 0x00, configMsg);
                SendModemConfigurationMessage(msg_ProgBlock);
                MOOSTrace("iModem: Sending first mtProgBlock : ");
                msg_ProgBlock.print_hex(300);
                m_uiTimeoutUS = 2000;
                m_serial_thread_tempo.Start();//A timeout of 2 seconds can be set. If the mtPgrAck is not received within this timeout period then re-send the mtEraseSector
            }
            else if(m_bGetFirstPgrAck && m_bGetSecondPgrAck && !m_bGetThirdPgrAck && m_uiTimeoutUS == 0)
            {
                //Send second mtProgBlock
                string configMsg;
                configMsg.append(128, 0x00);
                SeaNetMsg_ProgBlock msg_ProgBlock(m_modemNodeAddr, 0x01, configMsg);
                SendModemConfigurationMessage(msg_ProgBlock);
                MOOSTrace("iModem: Sending second mtProgBlock : ");
                msg_ProgBlock.print_hex(300);
                m_uiTimeoutUS = 2000;
                m_serial_thread_tempo.Start();//A timeout of 2 seconds can be set. If the mtPgrAck is not received within this timeout period then re-send the mtEraseSector
            }
            else if(m_bGetFirstPgrAck && m_bGetSecondPgrAck && m_bGetThirdPgrAck && !m_bModemConfiguratonComplete)
            {
                //Send mtReBoot
                SeaNetMsg_ReBoot msg_ReBoot(m_modemNodeAddr);
                SendModemConfigurationMessage(msg_ReBoot);
                MOOSTrace("iModem: Sending mtReBoot : ");
                msg_ReBoot.print_hex(200);
                m_bMtReBootHasBeenSent = true;
            }
        }
        else if(m_bModemConfiguratonComplete)
        {
            string modemRole = (m_BBUserData.at(49))?"slave":"master";
            MOOSTrace("iModem: Modem has been configured to %s\n",modemRole.c_str());
        }
        else
        {
            MOOSTrace("iModem: No mtAlive, check connexion.\n");
        }
    }
}

//------------------------------------------------------------
// Procedure: receiveMessage
bool Modem::receiveMessage(string &message, double reception_timeout)
{
    char message_char[MESSAGE_MAX_LENGTH];
    message = "";

    if(m_Port.ReadNWithTimeOut(message_char, MESSAGE_MAX_LENGTH,reception_timeout))
    {
        message = message_char;
        // Removing '\n' for display purposes
        //message.erase(remove(message.begin(), message.end(), '\r\n'), message.end());
        return true;
    }
    return false;
}


bool Modem::OnStartUp()
{
    setlocale(LC_ALL, "C");
    list<string> sParams;
    m_MissionReader.EnableVerbatimQuoting(false);

    if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
    {
        MOOSTrace("iModem: Reading configuration\n");
        list<string>::iterator p;
        for(p = sParams.begin() ; p != sParams.end() ; p++)
        {
            string original_line = *p;
            string param = stripBlankEnds(toupper(biteString(*p, '=')));
            string value = stripBlankEnds(*p);

            MOOSTrace(original_line);MOOSTrace("\n");

            if(param == "SERIAL_PORT_NAME")
            {
                MOOSTrace("iModem: Using %s serial port\n", value.c_str());
                m_portName = value;
            }
            else if(param == "BAUD_RATE_CONF")
            {
                m_baudrate_conf = atoi(value.c_str());
                MOOSTrace("iModem: serial port baud rate configured to %d\n", atoi(value.c_str()));
            }
            else if(param == "BAUD_RATE_COMM")
            {
                m_baudrate_comm = atoi(value.c_str());
                MOOSTrace("iModem: serial port baud rate configured to %d\n", atoi(value.c_str()));
            }
        }
    }
    else
        MOOSTrace("iModem: No configuration read.\n");

    MOOSTrace("iModem: Openning %s serial port at baud rate %d\n", m_portName.c_str(), m_baudrate_comm);
    bool portOpened = m_Port.Create(m_portName.c_str(), m_baudrate_comm);
    MOOSTrace("iModem: Openned\n");
    //m_Port.SetTermCharacter('\n');
    m_Port.Flush();
    m_timewarp = GetMOOSTimeWarp();
    RegisterVariables();

    return(true);
}

void Modem::RegisterVariables()
{
    Register("MODEM_GET_ROLE", 0);
    Register("MODEM_CONFIGURATION_REQUIRED", 0);
    Register("MODEM_SEND_MESSAGE", 0);
}
