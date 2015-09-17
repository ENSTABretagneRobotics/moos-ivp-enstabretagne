#--------------------------------------------------------
# The CMakeLists.txt for: pWallDetector
# Toutatis AUVs - ENSTA Bretagne
# Author(s): 
#--------------------------------------------------------

SET(SRC
  WallDetector.cpp
  main.cpp
)

ADD_EXECUTABLE(pWallDetector ${SRC})

TARGET_LINK_LIBRARIES(pWallDetector
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
