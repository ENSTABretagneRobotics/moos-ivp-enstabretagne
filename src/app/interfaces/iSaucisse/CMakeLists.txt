#--------------------------------------------------------
# The CMakeLists.txt for: iSaucisse
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Saucisse.cpp
  Pololu.cpp
  Nuc.cpp
  main.cpp
)

SET(CMAKE_AUTOMOC ON)

ADD_EXECUTABLE(iSaucisse ${SRC})

TARGET_LINK_LIBRARIES(iSaucisse
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
