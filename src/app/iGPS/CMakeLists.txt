#--------------------------------------------------------
# The CMakeLists.txt for: iGPS
# ENSTA Bretagne
# Author(s): Clément Aubry from MSIS
#--------------------------------------------------------

SET(SRC
  GPS.cpp
  main.cpp
)

ADD_EXECUTABLE(iGPS ${SRC})

TARGET_LINK_LIBRARIES(iGPS
   ${MOOS_LIBRARIES}
   ${MOOSGeodesy_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread)
