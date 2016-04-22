/************************************************************/
/*    FILE: Viewer.h
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Guilherme Schvarcz Franco
/*    DATE: 2016
/************************************************************/

#include "Viewer.h"

Viewer::Viewer():
    mThumbHeight(100.), thumbPadding(10.),
    mWindowName("Sensor Viewer"), activedView(""),
    screen(768, 1024, CV_8UC3), singleWindow(true), changeMode(true)
{
}

void Viewer::UpdateView(string imgName, Mat img)
{
    if (activedView == "")
        activedView = imgName;
    mapImages[imgName] = img;
}

void Viewer::RemoveView(string imgName)
{
    if(imgName == activedView)
        activedView = "";

    mapImages.erase(imgName);
}

void Viewer::Render()
{
    if(changeMode)
    {
        changeMode = false;
        destroyAllWindows();
        if (singleWindow)
        {
            imshow(mWindowName, screen);
            setMouseCallback(mWindowName,onMouse,this);
        }
    }
    if (!singleWindow)
    {
        for (map<string,Mat>::iterator it = mapImages.begin(); it != mapImages.end(); it++)
        {
            if (!it->second.empty())
                imshow(it->first, it->second);
        }
    }
    else
    {

        this->initializeRender();

        this->renderMainView();

        this->renderThumbs();

        imshow(mWindowName, screen);
    }
    keyboardHandle();
}

void Viewer::initializeRender()
{
    screen.setTo(0);

    //Auto select the actived view
    if (activedView == "" || mapImages[activedView].empty())
    {
        map<string, Mat>::iterator it;
        for(it = mapImages.begin();it->second.empty();it++);

        if(it != mapImages.end())
            activedView = it->first;
        else
            activedView = "";
    }
}

void Viewer::renderMainView()
{
    //Copy to the main screen
    if (activedView != "")
    {
        Mat img = mapImages[activedView].clone();

        if (!img.empty())
        {
            float factor = std::min((float)screen.rows/img.rows, (float)screen.cols/img.cols);
            resize(img,img,Size(factor*img.cols, factor*img.rows));
            convertImage(img);

            float xPos = (screen.cols - img.cols)/2;
            float yPos = (screen.rows - img.rows)/2;
            img.copyTo(screen(Rect(xPos, yPos, img.cols, img.rows)));
        }
    }
}

void Viewer::renderThumbs()
{
    //Create thumbs
    mapThumbsPositions.clear();
    int xPos = thumbPadding;
    for(map<string, Mat>::iterator it = mapImages.begin(); it != mapImages.end(); it++)
    {
        if (it->second.empty())
            continue;

        Mat img = it->second.clone();

        float mThumbWidth = img.cols*mThumbHeight/img.rows;

        Mat thumb;
        resize(img,thumb,Size(mThumbHeight, mThumbWidth));

        convertImage(thumb);

        Rect roi(xPos, screen.rows - thumbPadding - thumb.rows,thumb.cols, thumb.rows);
        mapThumbsPositions[it->first] = roi;

        thumb.copyTo(screen(roi));

        xPos += thumbPadding + mThumbWidth;
    }
}

void Viewer::convertImage(Mat &imgToConvert)
{
    if (imgToConvert.type() == CV_32FC1)
        imgToConvert *= 255;

    if (imgToConvert.channels() == 1)
        cvtColor(imgToConvert,imgToConvert,CV_GRAY2BGR);

    imgToConvert.convertTo(imgToConvert,screen.type());
}

void Viewer::keyboardHandle()
{
    int key = waitKey(1) & 255;

    string lastView = "";
    map<string, Mat>::iterator it;

    switch(key)
    {
    case 's':
    case 'S':
        singleWindow = !singleWindow;
        changeMode = true;
        break;
    case 27:
        exit(0);
        break;
    case 'w': //left
        for(it = mapImages.begin(); it != mapImages.end(); it++)
        {
            if (it->second.empty())
                continue;
            if(it->first == activedView)
                break;

            lastView = it->first;

        }
        if( lastView=="" && mapImages.size()>0)
        {
            it = mapImages.end();
            it--;
            lastView = it->first;
        }

        activedView = lastView;
        break;
    case 'x': //right
        for(it = mapImages.begin(); it != mapImages.end(); it++)
        {
            if (it->second.empty())
                continue;
            if(lastView == activedView)
            {
                activedView = it->first;
                break;
            }

            lastView = it->first;
        }

        if( it==mapImages.end())
        {
            for(it = mapImages.begin();it->second.empty();it++);

            if(it != mapImages.end())
                activedView = it->first;
        }
        break;
    }
}

//Just q wrap for Opencv
void Viewer::onMouse( int event, int x, int y, int, void* userdata )
{
    Viewer* viewer = reinterpret_cast<Viewer*>(userdata);
    viewer->onMouse(event,x,y);
}

void Viewer::onMouse( int event, int x, int y)
{
    if (event == CV_EVENT_LBUTTONUP)
    {
        //Discovery in which thumb did the user clicked.
        for(map<string,Rect>::iterator it = mapThumbsPositions.begin();it!= mapThumbsPositions.end(); it++)
        {
            Rect thumbPos = it->second;
            if( (thumbPos.x < x && thumbPos.x+thumbPos.width > x)
                && (thumbPos.y < y && thumbPos.y+thumbPos.height > y))
            {
                activedView = it->first;
                return;
            }
        }
    }
}
