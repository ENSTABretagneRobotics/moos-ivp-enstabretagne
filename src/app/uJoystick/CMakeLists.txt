#--------------------------------------------------------
# The CMakeLists.txt for: uJoystick
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Vincent Drevelle, Simon Rohou
#--------------------------------------------------------

SET(SRC
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
