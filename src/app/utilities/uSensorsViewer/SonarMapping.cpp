/************************************************************/
/*    FILE: SonarMapping.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#include "SonarMapping.h"

/**************************************************************
 *
 *                   SonarMapping by Scanline
 *
  ************************************************************/
SonarMappingScanline::SonarMappingScanline()
{
    mContrast = 1.0;
    mFOV = M_PI/180.;
    isConnected = false;
}

void SonarMappingScanline::Update(double bearing, std::vector<double> scanline)
{
    mBearing = bearing;
    mScanline.assign(scanline.begin(),scanline.end());
}

void SonarMappingScanline::Iterate()
{
    if (mScanline.size())
    {
        if(mImg.empty() || (mImg.rows != 2*mScanline.size())){
            mImg.create(2*mScanline.size(),2*mScanline.size(),CV_32FC(1));
            mImg.setTo(Scalar(1.0));
        }

        AddSector();
        DrawScanLineCurve();

        mScanline.clear();
    }
}

Mat SonarMappingScanline::SonarMap()
{
    return mImg;
}

void SonarMappingScanline::ApplyContrastAndNormalize()
{
    //Apply a constrast
    double mean = 0.0;
    for(int r=0; r<mScanline.size(); r++){
        mean += mScanline[r];
    }

    mean /= mScanline.size();

    for(int r=1; r<mScanline.size(); r++){
        mScanline[r] = (mScanline[r]-mean)*mContrast + mean;
    }

    //Normalize
    double maxValue = numeric_limits<double>::min(), minValue = numeric_limits<double>::max();

    for(int r=0; r<mScanline.size(); r++){
        maxValue = max(maxValue,mScanline[r]);
        minValue = min(minValue,mScanline[r]);
    }
    for(int r=0; r<mScanline.size(); r++){
        mScanline[r] = (mScanline[r]-minValue)/(maxValue-minValue);
    }
}

void SonarMappingScanline::AddSector()
{
    ApplyContrastAndNormalize();

    //To avoid gaps, run over all matrix
    for(int x = 0; x < mImg.cols; x++){
        for(int y = 0; y < mImg.rows; y++){
            double dY = y-mImg.rows/2., dX = x-mImg.cols/2.;
            double celAngle = atan2(dY, dX)+ M_PI;

            if ( abs(mBearing - celAngle) < mFOV )
            {
                double distance = sqrt((dX*dX) + (dY*dY));
                mImg.at<float>(y, x) = 1.0 - mScanline[ (int)round(distance) ];
            }
        }
    }
}

Mat SonarMappingScanline::ScanLineCurve()
{
    return mScanlineCurve;
}

void  SonarMappingScanline::DrawScanLineCurve()
{
    int strech = 2;
    if (mScanlineCurve.empty())
        mScanlineCurve.create(100, (int)mScanline.size()*strech,CV_8UC3);

    mScanlineCurve.setTo(0);
    for(double i = 1; i<mScanline.size(); i++)
    {
        line(mScanlineCurve,Point((i-1)*strech,mScanlineCurve.rows - mScanline.at(i-1)),Point(i*strech,mScanlineCurve.rows - mScanline.at(i)),Scalar(255,0,0),1);
    }
}

string SonarMappingScanline::toString()
{
    ACTable actab(4);
    actab << "Connected | FOV | Contrast | LastBearing";
    actab.addHeaderLines();
    actab << (isConnected?"true":"false") << mFOV << mContrast << mBearing;
    return actab.getFormattedString();
}



/**************************************************************
 *
 *                   SonarMapping by Distance
 *
  ************************************************************/

SonarMappingDistance::SonarMappingDistance()
{
    isConnected = false;
    mScanLength = 144;
    mDistance = -1;
}

void SonarMappingDistance::Update(double bearing, double distance)
{
    mBearing = bearing;
    mDistance = distance;
}

void SonarMappingDistance::Iterate()
{
    if (mDistance != -1)
    {
        if(mImg.empty() || (mImg.rows != 2*mScanLength)){
            mImg.create(2*mScanLength,2*mScanLength,CV_32FC(1));
            mImg.setTo(Scalar(1.0));
        }
        int x = round(mImg.cols/2 + mDistance*cos(mBearing));
        int y = round(mImg.rows/2 + mDistance*sin(mBearing));

        if ((x<mImg.cols) && (y<mImg.rows))
            mImg.at<float>(y,x) = 0;
    }
}

Mat SonarMappingDistance::SonarMap()
{
    return mImg;
}


string SonarMappingDistance::toString()
{
    ACTable actab(4);
    actab << "Connected | ScanLength | LastBearing | LastDistance";
    actab.addHeaderLines();
    actab << (isConnected?"true":"false") << mScanLength << mBearing << mDistance;
    return actab.getFormattedString();
}
