#--------------------------------------------------------
# The CMakeLists.txt for: uNavScreen
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  NavScreen.cpp
  main.cpp
)

ADD_EXECUTABLE(uNavScreen ${SRC})

TARGET_LINK_LIBRARIES(uNavScreen
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
