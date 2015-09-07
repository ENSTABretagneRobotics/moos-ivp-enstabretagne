#ifndef VIDEOGRABV4L2_H
#define VIDEOGRABV4L2_H

#include <string>
#include <opencv/cv.h>

class VideoGrabV4L2
{
  // Structs
  struct buffer {
    void   *start;
    size_t length;
  };
  // Private members
  int            fd;
  unsigned int   n_buffers;
  std::string    dev_name;
  struct buffer  *buffers;
  //int            width, height;

  IplImage 	 frame;
  // Private functions
  void xioctl(int fh, int request, void *arg);

public:
  VideoGrabV4L2();
  virtual ~VideoGrabV4L2();
  bool open(const std::string& device_name="/dev/video0", int width=640, int height=480);
//      CV_WRAP virtual bool isOpened() const;
  void release();
//    CV_WRAP virtual bool grab();
//    CV_WRAP virtual bool retrieve(CV_OUT Mat& image, int channel=0);
//    virtual VideoCapture& operator >> (CV_OUT Mat& image);
  bool read(cv::Mat& image);
//  CV_WRAP virtual bool set(int propId, double value);
//  CV_WRAP virtual double get(int propId);
};

#endif // VIDEOGRABV4L2_H
