#--------------------------------------------------------
# The CMakeLists.txt for: iPololu
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Pololu.cpp
  PololuPin.cpp
  PololuPinOut.cpp
  PololuApp.cpp
  main.cpp
)

SET(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5Widgets)

ADD_EXECUTABLE(iPololu ${SRC})

TARGET_LINK_LIBRARIES(iPololu
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   Qt5::Widgets)
