#--------------------------------------------------------
# The CMakeLists.txt for: pWallDetector
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): 
#--------------------------------------------------------

SET(SRC
  WallDetector.cpp
  main.cpp
  SonarWall.cpp
)

ADD_EXECUTABLE(pWallDetector ${SRC})

TARGET_LINK_LIBRARIES(pWallDetector
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
