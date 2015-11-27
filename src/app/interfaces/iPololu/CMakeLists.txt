#--------------------------------------------------------
# The CMakeLists.txt for: iPololu
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Pololu.cpp
  PololuPin.cpp
  PololuPinIn.cpp
  PololuPinOut.cpp
  PololuApp.cpp
  main.cpp
)

SET(CMAKE_AUTOMOC ON)

ADD_EXECUTABLE(iPololu ${SRC})

TARGET_LINK_LIBRARIES(iPololu
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
