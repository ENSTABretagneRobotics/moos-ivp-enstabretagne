#--------------------------------------------------------
# The CMakeLists.txt for:                       uSensorsViewer
# Author(s):                              
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  SensorViewer.cpp
  main.cpp
)

ADD_EXECUTABLE(uSensorsViewer ${SRC})

TARGET_LINK_LIBRARIES(uSensorsViewer
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   opencv_core
   opencv_imgproc
   opencv_highgui
)