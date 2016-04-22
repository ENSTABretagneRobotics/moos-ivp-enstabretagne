/************************************************************/
/*    FILE: Viewer.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#ifndef VIEWER_H
#define VIEWER_H

#include <map>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class Viewer
{
public:
    Viewer();
    void UpdateView(string imgName, Mat img);
    void RemoveView(string imgName);
    void Render();

private:
    Mat screen;
    map<string, Mat> mapImages;
    map<string, Rect> mapThumbsPositions;
    string activedView, mWindowName;
    float mThumbHeight, thumbPadding;
    bool singleWindow, changeMode;

    void convertImage(Mat &imgToConvert);
    void initializeRender();
    void renderMainView();
    void renderThumbs();
    void keyboardHandle();
    static void onMouse( int event, int x, int y, int, void* );
    void onMouse( int event, int x, int y);
};

#endif // VIEWER_H
