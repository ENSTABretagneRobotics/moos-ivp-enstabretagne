/************************************************************/
/*    FILE: SensorViewer.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#include "SensorViewer.h"

SensorViewer::SensorViewer()
{

}

SensorViewer::~SensorViewer()
{
    destroyAllWindows();
}

bool SensorViewer::OnNewMail(MOOSMSG_LIST &NewMail)
{
    AppCastingMOOSApp::OnNewMail(NewMail);
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

        // **************** CAMERA DATA ************************
        if(  mapCameras.find(msg.GetKey()) != mapCameras.end())
        {
            CameraViewer img = mapCameras[msg.GetKey()];
            img.UpdateBinaryData(msg.GetBinaryData());
            mapCameras[msg.GetKey()] = img;
        }

        // **************** SONAR DATA ************************
        else if(msg.GetKey().find("_RAW_DATA") != string::npos)
        {
            double bearing;
            MOOSValFromString(bearing, msg.GetString(), "bearing", true);
            bearing = MOOSDeg2Rad(bearing);

            int nRows, nCols;
            std::vector<double> scanline;
            MOOSVectorFromString(msg.GetString(), scanline, nRows, nCols);

            string key = msg.GetKey().substr(0,msg.GetKey().find("_RAW_DATA"));
            if(mapSonarsScanline.find(key) != mapSonarsScanline.end())
            {
               mapSonarsScanline[key].Update(bearing, scanline);
            }

        }
        else if(msg.GetKey().find("_DISTANCE") != string::npos)
        {
            double bearing, distance;
            MOOSValFromString(bearing, msg.GetString(), "bearing", true);
            MOOSValFromString(distance, msg.GetString(), "distance", true);
            bearing = MOOSDeg2Rad(bearing);


            string key = msg.GetKey().substr(0,msg.GetKey().find("_DISTANCE"));
            if(mapSonarsDistance.find(key) != mapSonarsDistance.end())
            {
               mapSonarsDistance[key].Update(bearing,distance);
            }

        }
        else if(msg.GetKey().find("_CONNECTED") != string::npos)
        {
            string key = msg.GetKey().substr(0,msg.GetKey().find("_CONNECTED"));
            if(mapSonarsScanline.find(key) != mapSonarsScanline.end())
            {
                mapSonarsScanline[key].isConnected = (msg.GetString() == "true");
                mapSonarsDistance[key].isConnected = (msg.GetString() == "true");
            }

        }

        // **************** MAPPING PARAMS ************************
        else if(msg.GetKey().find("_CONTRAST") != string::npos)
        {
            string key = msg.GetKey().substr(0,msg.GetKey().find("_CONTRAST"));
            if(mapSonarsScanline.find(key) != mapSonarsScanline.end())
            {
               mapSonarsScanline[key].mContrast = msg.GetDouble();
            }

        }
        else if(msg.GetKey().find("_SCAN_LENGTH") != string::npos)
        {
            string key = msg.GetKey().substr(0,msg.GetKey().find("_SCAN_LENGTH"));
            if(mapSonarsDistance.find(key) != mapSonarsDistance.end())
            {
               mapSonarsDistance[key].mScanLength = (int)msg.GetDouble();
            }
        }
        else if(msg.GetKey().find("_FOV") != string::npos)
        {
            string key = msg.GetKey().substr(0,msg.GetKey().find("_FOV"));
            if(mapSonarsScanline.find(key) != mapSonarsScanline.end())
            {
               mapSonarsScanline[key].mFOV = MOOSDeg2Rad(msg.GetDouble());
            }
        }

    }
    return(true);
}

bool SensorViewer::OnConnectToServer()
{
    RegisterVariables();
    return(true);
}

bool SensorViewer::Iterate()
{
    AppCastingMOOSApp::Iterate();


    //Iterate sonars
    for( map<string,SonarMappingScanline>::iterator it = mapSonarsScanline.begin(); it != mapSonarsScanline.end();it++)
    {
        it->second.Iterate();
    }
    for( map<string,SonarMappingDistance>::iterator it = mapSonarsDistance.begin(); it != mapSonarsDistance.end();it++)
    {
        it->second.Iterate();
    }


    //Show everything
    for( map<string,CameraViewer>::iterator it = mapCameras.begin(); it != mapCameras.end();it++)
    {
        mViewer.UpdateView(it->first, it->second.mImg);
    }
    for( map<string,SonarMappingScanline>::iterator it = mapSonarsScanline.begin(); it != mapSonarsScanline.end();it++)
    {
        mViewer.UpdateView(it->first+"_SCANLINE", it->second.SonarMap());
    }
    for( map<string,SonarMappingDistance>::iterator it = mapSonarsDistance.begin(); it != mapSonarsDistance.end();it++)
    {
        mViewer.UpdateView(it->first+"_DISTANCE", it->second.SonarMap());
    }

    mViewer.Render();
    waitKey(1);
    AppCastingMOOSApp::PostReport();
    return(true);
}

bool SensorViewer::OnStartUp() {
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    m_MissionReader.EnableVerbatimQuoting(false);
    string lastSonar = "";
    string lastImage = "";
    if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
    {
        STRING_LIST::iterator p;
        for (p = sParams.begin(); p != sParams.end(); p++) {
            string orig = *p;
            string line = *p;
            string param = toupper(biteStringX(line, '='));
            string value = line;
            bool handled = false;

            cout << param << " - " << value << endl;
            if (param.find("IMAGE_VAR_NAME") == 0)
            {
                if (mapCameras.find(value) == mapCameras.end())
                {
                    mapCameras.insert ( std::pair<string, CameraViewer> (value,CameraViewer()) );
                    lastImage = value;
                    handled = true;
                }
            }
            else if (param == "IMAGE_WIDTH")
            {
                if (lastImage != "")
                {
                    mapCameras[lastImage].mWidth = atoi((char*)value.c_str());
                    handled = true;
                }
            }
            else if (param == "IMAGE_HEIGHT")
            {
                if (lastImage != "")
                {
                    mapCameras[lastImage].mHeight = atoi((char*)value.c_str());
                    handled = true;
                }
            }
            else if (param == "IMAGE_DEPTH")
            {
                if (lastImage != "")
                {

                    if(value.c_str() == "8U")
                    {
                        mapCameras[lastImage].mType = CV_8U;
                        handled = true;
                    }
                    if(value.c_str() == "8UC1")
                    {
                        mapCameras[lastImage].mType = CV_8UC1;
                        handled = true;
                    }
                    if(value.c_str() == "8UC3")
                    {
                        mapCameras[lastImage].mType = CV_8UC3;
                        handled = true;
                    }

                    if(value.c_str() == "32F")
                    {
                        mapCameras[lastImage].mType = CV_32F;
                        handled = true;
                    }
                    if(value.c_str() == "32FC1")
                    {
                        mapCameras[lastImage].mType = CV_32FC1;
                        handled = true;
                    }
                    if(value.c_str() == "32FC3")
                    {
                        mapCameras[lastImage].mType = CV_32FC3;
                        handled = true;
                    }

                }
            }
            else if (param == "SONAR_VAR_NAME")
            {
                if (mapSonarsScanline.find(value) == mapSonarsScanline.end())
                {
                    mapSonarsScanline.insert ( std::pair<string, SonarMappingScanline> (value,SonarMappingScanline()) );
                    mapSonarsDistance.insert ( std::pair<string, SonarMappingDistance> (value,SonarMappingDistance()) );
                    lastSonar = value;
                    handled = true;
                }
            }

            else if (param == "SONAR_SCAN_LENGTH")
            {
                if (lastSonar != "")
                {
                    mapSonarsDistance[lastSonar].mScanLength = atoi((char*)value.c_str());
                    handled = true;
                }
            }
            else if (param == "SONAR_FOV")
            {
                if (lastSonar != "")
                {
                    mapSonarsScanline[lastSonar].mFOV = MOOSDeg2Rad(atof((char*)value.c_str()));
                    handled = true;
                }
            }
            else if (param == "SONAR_CONTRAST")
            {
                if (lastSonar != "")
                {
                    mapSonarsScanline[lastSonar].mContrast = atof((char*)value.c_str());
                    handled = true;
                }
            }

            if (!handled)
                reportUnhandledConfigWarning(orig);
        }
    }

    registerVariables();

    return(true);
}

void SensorViewer::registerVariables()
{
    AppCastingMOOSApp::RegisterVariables();

    // CAMERA DATA

    for( map<string,CameraViewer>::iterator it = mapCameras.begin(); it != mapCameras.end();it++)
    {
        Register(it->first, 0);
    }

    for( map<string,SonarMappingScanline>::iterator it = mapSonarsScanline.begin(); it != mapSonarsScanline.end();it++)
    {
        Register(it->first+"_RAW_DATA", 0);
        Register(it->first+"_DISTANCE", 0);
        Register(it->first+"_CONNECTED", 0);
        Register(it->first+"_CONTRAST", 0);
        Register(it->first+"_FOV", 0);
        Register(it->first+"_SCAN_LENGTH", 0);
    }

}

bool SensorViewer::buildReport()
{
#if 0 // Keep these around just for template
    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
#endif

    for( map<string,CameraViewer>::iterator it = mapCameras.begin(); it != mapCameras.end();it++)
    {
        m_msgs <<  it->first << std::endl;
        m_msgs <<  it->second.toString() << std::endl << std::endl;
    }

    m_msgs << std::endl;
    for( map<string,SonarMappingScanline>::iterator it = mapSonarsScanline.begin(); it != mapSonarsScanline.end();it++)
    {
        m_msgs <<  it->first << std::endl;
        m_msgs <<  it->second.toString() << std::endl << std::endl;
    }
    m_msgs << std::endl;
    for( map<string,SonarMappingDistance>::iterator it = mapSonarsDistance.begin(); it != mapSonarsDistance.end();it++)
    {
        m_msgs <<  it->first << std::endl;
        m_msgs <<  it->second.toString() << std::endl << std::endl;
    }

    return(true);
}
