#--------------------------------------------------------
# The CMakeLists.txt for: iSounder
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Sounder.cpp
  main.cpp
)

ADD_EXECUTABLE(iSounder ${SRC})

TARGET_LINK_LIBRARIES(iSounder
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
