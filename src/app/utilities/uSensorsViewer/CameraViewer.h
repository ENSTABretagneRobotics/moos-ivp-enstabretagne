/************************************************************/
/*    FILE: CameraViewer.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#ifndef CAMERAVIEWER_H
#define CAMERAVIEWER_H

#include <opencv/cv.h>
#include "ACTable.h"

using namespace cv;

class CameraViewer
{
public:
    CameraViewer();
    void UpdateInfoData();
    void UpdateBinaryData(unsigned char *data);
    string toString();
    Mat mImg;
    int mWidth, mHeight, mType;

private:
    Mat mCameraParams;
};

#endif // CAMERAVIEWER_H
