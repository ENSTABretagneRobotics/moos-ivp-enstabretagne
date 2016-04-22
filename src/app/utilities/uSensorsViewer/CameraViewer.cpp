/************************************************************/
/*    FILE: CameraViewer.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#include "CameraViewer.h"

CameraViewer::CameraViewer()
{
    mType = CV_8UC(3);
    mHeight = 360;
    mWidth = 288;
}

void CameraViewer::UpdateBinaryData(unsigned char *data)
{
    if (mImg.empty())
    {
        if ((mHeight != 0) && (mWidth != 0))
        {
           mImg.create(mHeight,mWidth,mType);
           mImg.setTo(0);
        }
        else
            return;
    }

    memcpy(mImg.data, data, mImg.rows*mImg.step);
}

string CameraViewer::toString()
{
    ACTable actab(3);
    actab << "Width | Height | Type";
    actab.addHeaderLines();
    actab << mHeight << mHeight << mType;
    return actab.getFormattedString();
}
