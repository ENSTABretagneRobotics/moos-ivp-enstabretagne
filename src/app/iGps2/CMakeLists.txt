#--------------------------------------------------------
# The CMakeLists.txt for: iGps2
# Toutatis AUVs - ENSTA Bretagne
# Author(s): Thomas Le Mezo
#--------------------------------------------------------

SET(SRC
  Gps2.cpp
  main.cpp
)

ADD_EXECUTABLE(iGps2 ${SRC})

TARGET_LINK_LIBRARIES(iGps2
   ${MOOS_LIBRARIES}
   xml-documentation
   apputil
   mbutil
   m
   pthread
   nmea
   boost_system
   proj)
