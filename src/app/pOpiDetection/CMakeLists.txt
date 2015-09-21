#--------------------------------------------------------
# The CMakeLists.txt for: pOpiDetection
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Guilherme Schvarcz Franco, Simon Rohou
#--------------------------------------------------------

SET(SRC
  OpiDetection.cpp
  main.cpp
)

ADD_EXECUTABLE(pOpiDetection ${SRC})

TARGET_LINK_LIBRARIES(pOpiDetection
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   opencv_core
   opencv_imgproc
   opencv_highgui
   opencv_video
   opencv_legacy
   pthread)
