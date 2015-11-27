#--------------------------------------------------------
# The CMakeLists.txt for: pController
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): MDS
#--------------------------------------------------------

SET(SRC
  Controller.cpp
  main.cpp
)

ADD_EXECUTABLE(pController ${SRC})

TARGET_LINK_LIBRARIES(pController
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
