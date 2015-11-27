/************************************************************/
/*    FILE: SonarCSVPlayer.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Clément Aubry
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "string.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "SonarCSVPlayer.h"


using namespace std;

//---------------------------------------------------------
// Constructor

SonarCSVPlayer::SonarCSVPlayer()
{
  m_sFilename = "";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SonarCSVPlayer::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "FOO")
      cout << "great!";

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool SonarCSVPlayer::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool SonarCSVPlayer::Iterate()
{
  AppCastingMOOSApp::Iterate();

  string ligne;
  char* pChar;
  if (getline( buffer, ligne ))
  {
    // cout<< "line : " << ligne <<endl;
    Notify("SONAR_CSV_LINE", ligne);
    //SOf,DateTime,Node,Status,Hdctrl,Rangescale,Gain,Slope,AdLow,AdSpan,LeftLim,RightLim,Steps,Bearing,Dbytes,Dbytes of DATA
    //SON,14:23:52.021,2,16,8963,750,105,125,9,79,0,0,32,3200,400,107,105,93,49,9,1,1,1,1,1,1,1,1,1,4,1,2,2,1,1,6,6,2,1,1,1,2,2,5,1,...

    /**
    *
    * This code decode a scanline to put it in a typedef defined in SonarCSVPlayer.h
    * USELESS here cause the line had to be sent as a string
    * Could be usefull decoding scanline for treatment
    */
    char lineChar[9000];
    sprintf(lineChar,"%s",ligne.c_str());
    stringstream ss_scanline;
    // ss_scanline << '{';

    ScanLine scanLine;
    int k=0;
    string tok = ",\r\n";
    pChar = strtok (lineChar,tok.c_str());
    while (pChar != NULL)
    {
      Notify("SONAR_PCHAR",pChar);
      switch (k)
      {
        case 0:scanLine.SOf = pChar;break;
        case 1:scanLine.dateTime = pChar;break;
        case 2:scanLine.node = atoi(pChar);break;
        case 3:scanLine.status = atoi(pChar);break;
        case 4:scanLine.hdrCtrl = atoi(pChar);break;
        case 5:scanLine.rangeScale = atoi(pChar);break;
        case 6:scanLine.gain = atoi(pChar);break;
        case 7:scanLine.slope = atoi(pChar);break;
        case 8:scanLine.adLow = atoi(pChar);break;
        case 9:scanLine.adSpan = atoi(pChar);break;
        case 10:scanLine.LeftLim = atoi(pChar);break;
        case 11:scanLine.RightLim = atoi(pChar);break;
        case 12:scanLine.Steps = atoi(pChar);break;
        case 13:scanLine.Bearing = atoi(pChar);break;
        case 14:scanLine.Dbytes = atoi(pChar);break;
        default:
          ss_scanline << pChar;
          if (k < scanLine.Dbytes+14)
            ss_scanline << ',';
          break;
      }
      pChar = strtok (NULL, tok.c_str());
      k++;
    }
    // ss_scanline << '}';bui
    double adinterval = ((2*scanLine.rangeScale / scanLine.Dbytes) / 1500) / 640.e-9 ;
    sprintf(lineChar,"bearing=%d,ad_interval=%lf,scanline=%s",scanLine.Bearing, adinterval, ss_scanline.str().c_str());
    Notify("SONAR_RAW_DATA", lineChar);
    Notify("SONAR_SCANLINE", ss_scanline.str());
    // // ***************************************
    // // MOSSDB data for WALL DETECTOR
    Notify("SONAR_BEARING", scanLine.Bearing);

    // for (int k=0; k<pHdta->nBins(); k++){
    //   ss_scanline << pHdta->scanlineData()[k];
    //   if(k!=pHdta->nBins()-1)
    //     ss_scanline << ',';
    // }
    // ss_scanline << '}';
    // Notify("SONAR_SCANLINE", ss_scanline.str());
    // // ***************************************


  }

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SonarCSVPlayer::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

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

    if(param == "SONAR_CSV_FILE")
    {
      m_sFilename = value;
      handled = true;
    }
    else if(param == "BAR")
    {
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  //WORKING
  // ifstream ifs( "/home/clement/ENSTABZH/moos-ivp-toutatis/data/test.txt" );
  // ifstream ifs( "/home/clement/ENSTABZH/moos-ivp-toutatis/data/Seanet0_2014-10-03_14h23min51s.csv" );
  ifstream ifs( m_sFilename.c_str() );
  if ( ! ifs.is_open() ) {
    cout <<" Failed to open" << endl;
  }
  else {
    cout <<"Opened OK" << endl;
    buffer << ifs.rdbuf();
    std::cout << "Taille du buffer : " << buffer.str().size() << endl;
    ifs.close();
      string headerLine;
  getline(buffer, headerLine);
cout<< "header line : " << headerLine <<endl;

    // std::string ligne;
    // while ( std::getline( buffer, ligne ) )
    // {
    //     std::cout << ligne << std::endl;
    // }
  }

  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void SonarCSVPlayer::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("SONAR_POLL", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool SonarCSVPlayer::buildReport()
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
