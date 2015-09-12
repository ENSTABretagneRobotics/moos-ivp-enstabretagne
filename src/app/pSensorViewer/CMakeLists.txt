#--------------------------------------------------------
# The CMakeLists.txt for:                       pSensorViewer
# Author(s):                              
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  SensorViewer.cpp
  main.cpp
)

ADD_EXECUTABLE(pSensorViewer ${SRC})

TARGET_LINK_LIBRARIES(pSensorViewer
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