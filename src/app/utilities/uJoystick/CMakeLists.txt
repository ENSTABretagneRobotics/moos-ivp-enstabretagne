#--------------------------------------------------------
# The CMakeLists.txt for: uJoystick
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Vincent Drevelle, Simon Rohou
#--------------------------------------------------------

SET(SRC
  JoystickControl.cpp
  Joystick.cpp
  main.cpp
)

ADD_EXECUTABLE(uJoystick ${SRC})

TARGET_LINK_LIBRARIES(uJoystick
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
