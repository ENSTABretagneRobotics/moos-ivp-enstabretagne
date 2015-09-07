#--------------------------------------------------------
# The CMakeLists.txt for:                       pSensorViewer
# Author(s):                              
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  SensorViewer.cpp
  SensorViewer_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pSensorViewer ${SRC})

TARGET_LINK_LIBRARIES(pSensorViewer
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread
   opencv_core
   opencv_imgproc
   opencv_highgui
   # Our libraries
   Ciscrea_Images)
