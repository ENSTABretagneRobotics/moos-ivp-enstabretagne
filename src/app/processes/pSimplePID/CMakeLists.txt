#--------------------------------------------------------
# The CMakeLists.txt for: pSimplePID
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Thomas Le Mezo
#--------------------------------------------------------

SET(SRC
  SimplePID.cpp
  main.cpp
)

ADD_EXECUTABLE(pSimplePID ${SRC})

TARGET_LINK_LIBRARIES(pSimplePID
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
