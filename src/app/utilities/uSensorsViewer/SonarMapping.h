/************************************************************/
/*    FILE: SonarMapping.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#ifndef SONARMAPPING_H
#define SONARMAPPING_H

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <opencv/cv.h>
#include "ACTable.h"

using namespace std;
using namespace cv;

class SonarMappingScanline
{
public:
    SonarMappingScanline();

    void Update(double bearing, std::vector<double> scanline);
    void Iterate();
    Mat SonarMap();
    Mat ScanLineCurve();
    string toString();
    double mContrast, mFOV;
    bool isConnected;

private:
    cv::Mat mImg;
    cv::Mat mScanlineCurve;
    std::vector<double> mScanline;
    double mBearing;


    void DrawScanLineCurve();
    void AddSector();
    void ApplyContrastAndNormalize();
    void ShowScanline();
};


class SonarMappingDistance
{
public:
    SonarMappingDistance();

    void Update(double bearing, double distance);
    void Iterate();
    Mat SonarMap();
    string toString();
    bool isConnected;
    int mScanLength;

private:
    cv::Mat mImg;
    double mBearing, mDistance;
};

#endif // SONARMAPPING_H
