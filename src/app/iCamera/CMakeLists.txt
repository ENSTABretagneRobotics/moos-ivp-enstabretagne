#--------------------------------------------------------
# The CMakeLists.txt for:                       iCamera
# Author(s):                              Équipe Ciscrea - Veni Vidi Vici
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  videograbv4l2.cpp
  Camera.cpp
  main.cpp
)

ADD_EXECUTABLE(iCamera ${SRC})

TARGET_LINK_LIBRARIES(iCamera
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   v4l2
   opencv_core
   opencv_imgproc
   opencv_highgui
   opencv_video
   opencv_legacy
)