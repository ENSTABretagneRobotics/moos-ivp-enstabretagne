#--------------------------------------------------------
# The CMakeLists.txt for: pLatLon2LocalGrid
# ENSTA Bretagne Robotics - moos-ivp-enstabretagne
# Author(s): 
#--------------------------------------------------------

SET(SRC
  LatLon2LocalGrid.cpp
  main.cpp
)

ADD_EXECUTABLE(pLatLon2LocalGrid ${SRC})

TARGET_LINK_LIBRARIES(pLatLon2LocalGrid
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
