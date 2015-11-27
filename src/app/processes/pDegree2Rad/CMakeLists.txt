#--------------------------------------------------------
# The CMakeLists.txt for: pDegree2Rad
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): Simon Rohou
#--------------------------------------------------------

SET(SRC
  Degree2Rad.cpp
  main.cpp
)

ADD_EXECUTABLE(pDegree2Rad ${SRC})

TARGET_LINK_LIBRARIES(pDegree2Rad
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   geometry
   m
   pthread)
