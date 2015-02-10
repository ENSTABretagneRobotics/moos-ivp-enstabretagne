#--------------------------------------------------------
# The CMakeLists.txt for: uJoystick
# ENSTA Bretagne
# Author(s): Vincent Drevelle, Simon Rohou
#--------------------------------------------------------

SET(SRC
  Joystick.cpp
  Joystick_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(uJoystick ${SRC})

TARGET_LINK_LIBRARIES(uJoystick
   ${MOOS_LIBRARIES}
   ensta-documentation
   apputil
   mbutil
   m
   pthread)
