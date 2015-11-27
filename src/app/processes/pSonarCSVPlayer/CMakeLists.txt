#--------------------------------------------------------
# The CMakeLists.txt for: pSonarCSVPlayer
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Clément Aubry
#--------------------------------------------------------

SET(SRC
  SonarCSVPlayer.cpp
  main.cpp
)

ADD_EXECUTABLE(pSonarCSVPlayer ${SRC})

TARGET_LINK_LIBRARIES(pSonarCSVPlayer
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
