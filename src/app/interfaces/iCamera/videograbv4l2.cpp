/* V4L2 video picture grabber
   Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@infradead.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#include "videograbv4l2.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

VideoGrabV4L2::VideoGrabV4L2()
  : fd(-1), n_buffers(0)
{
  frame.width = 0;
  frame.height = 0;
  frame.imageData = 0;
}

VideoGrabV4L2::~VideoGrabV4L2()
{
  release();
}

bool VideoGrabV4L2::open(const std::string& dev_name, int width, int height)
{
  if (fd >= 0) {
    printf("Closing previously opened video device.");
    release();
  }
  
  struct v4l2_format              fmt;
  struct v4l2_buffer              buf;
  struct v4l2_requestbuffers      req;
  
  fd = v4l2_open(dev_name.c_str(), O_RDWR | O_NONBLOCK, 0);
  if (fd < 0) {
    perror("Cannot open device");
    return false;
  }

  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = width;
  fmt.fmt.pix.height      = height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  xioctl(fd, VIDIOC_S_FMT, &fmt);
  if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_BGR24) {
    printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");
    release();
    return false;
  }
  if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
    printf("Warning: driver is sending image at %dx%d\n",
      fmt.fmt.pix.width, fmt.fmt.pix.height);
  
  // Store image dimensions in image header
  cvInitImageHeader( &frame,
              cvSize( fmt.fmt.pix.width,
                  fmt.fmt.pix.height ),
              IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
  // Allocate memory for image data
  frame.imageData = new char[fmt.fmt.pix.sizeimage];
  if (!frame.imageData) {
    perror("Not enough memory to allocate image.");
    release();
    return false;
  }

  CLEAR(req);
  req.count = 2;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  xioctl(fd, VIDIOC_REQBUFS, &req);

  buffers = (buffer*)calloc(req.count, sizeof(*buffers));
  for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
    CLEAR(buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = n_buffers;

    xioctl(fd, VIDIOC_QUERYBUF, &buf);

    buffers[n_buffers].length = buf.length;
    buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
      PROT_READ | PROT_WRITE, MAP_SHARED,
      fd, buf.m.offset);

    if (MAP_FAILED == buffers[n_buffers].start) {
      perror("mmap");
      release();
      return false;
    }
  }

  for (unsigned int i = 0; i < n_buffers; ++i) {
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    xioctl(fd, VIDIOC_QBUF, &buf);
  }
  
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_STREAMON, &type);
  
  return true;
}

void VideoGrabV4L2::release()
{
  delete[] frame.imageData;
  frame.imageData = 0;
  frame.width = 0; frame.height = 0;
  
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_STREAMOFF, &type);
  
  for (unsigned int i = 0; i < n_buffers; ++i)
    v4l2_munmap(buffers[i].start, buffers[i].length);
  n_buffers = 0;
  
  v4l2_close(fd);
  fd = -1;
}

bool VideoGrabV4L2::read(cv::Mat& image)
{
  if (fd < 0) {
    perror("Video device not opened");
    return false;
  }
  
  fd_set fds;
  struct timeval tv;
  int r = -1;

  do {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    /* Timeout. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    r = select(fd + 1, &fds, NULL, NULL, &tv);
  } while ((r == -1 && (errno == EINTR)));
  if (r == -1) {
    perror("select failed (timeout)");
    return false;
  }

  struct v4l2_buffer buf;
  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  xioctl(fd, VIDIOC_DQBUF, &buf);

  // Resize grab image if needed
  /*  if(((unsigned long)capture->frame.width != capture->form.fmt.pix.width)
       || ((unsigned long)capture->frame.height != capture->form.fmt.pix.height)) {
        cvFree(&capture->frame.imageData);
        cvInitImageHeader( &capture->frame,
              cvSize( capture->form.fmt.pix.width,
                  capture->form.fmt.pix.height ),
              IPL_DEPTH_8U, 3, IPL_ORIGIN_TL, 4 );
       capture->frame.imageData = (char *)cvAlloc(capture->frame.imageSize);
*/

  // Copy grabbed frame to image data
  if (buffers[buf.index].start)
    memcpy((void*)frame.imageData, 
	   buffers[buf.index].start, 
	   frame.imageSize/*buf.bytesused*/);
  
  image = cv::Mat(&frame);
  
  xioctl(fd, VIDIOC_QBUF, &buf);
  
  return true;
}


void VideoGrabV4L2::xioctl(int fh, int request, void *arg)
{
        int r = -1;

        do {
                r = v4l2_ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}
